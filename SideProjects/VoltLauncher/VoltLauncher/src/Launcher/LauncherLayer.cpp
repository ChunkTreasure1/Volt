#define CURL_STATICLIB

#include "LauncherLayer.h"

#include "FileSystem.h"
#include "UIUtility.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "../vendor/miniz.h"

#include <Windows.h>
#include <Shlobj.h>

#include <curl/curl.h>


inline static std::atomic_bool g_isRunning = false;
inline static std::atomic<float> g_installProgress = 0.f;

namespace Utility
{
	static size_t WriteCallback(void* contents, size_t size, size_t nemb, void* userPtr)
	{
		size_t totalSize = size * nemb;
		FILE* file = (FILE*)userPtr;

		if (!g_isRunning)
		{
			return 0;
		}

		return fwrite(contents, 1, totalSize, file);
	}

	int ProgressBar(void* bar, double t, double d)
	{
		if (t != 0.0)
		{
			g_installProgress = float(d / t) * 0.5f;
		}

		return 0;
	}

	void DownloadFile(const std::string& url, const std::filesystem::path& targetPath, std::function<int32_t(void* bar, double t, double d)> progressFunction = nullptr)
	{
		CURL* curl;
		FILE* file;

		CURLcode res;

		curl = curl_easy_init();

		if (!curl)
		{
			fprintf(stderr, "Error initializing libcurl");
			return;
		}

		file = fopen(targetPath.string().c_str(), "wb");
		if (!file)
		{
			fprintf(stderr, "Error opening file for writing\n");
			curl_easy_cleanup(curl);
			return;
		}

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION);

		if (progressFunction)
		{
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressBar);
		}

		res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

		res = curl_easy_perform(curl);

		fclose(file);
		curl_easy_cleanup(curl);
	}

	void UnzipFile(const std::filesystem::path& srcPath, const std::filesystem::path& dstDir)
	{
		mz_zip_archive zipArchive;
		memset(&zipArchive, 0, sizeof(zipArchive));

		if (!mz_zip_reader_init_file(&zipArchive, srcPath.string().c_str(), 0))
		{
			return;
		}

		int32_t numFiles = mz_zip_reader_get_num_files(&zipArchive);

		float progressAdv = 0.5f / numFiles;

		for (int32_t i = 0; i < numFiles; i++)
		{
			g_installProgress = g_installProgress + progressAdv;

			mz_zip_archive_file_stat fileState;
			if (!mz_zip_reader_file_stat(&zipArchive, i, &fileState))
			{
				continue;
			}

			if (mz_zip_reader_is_file_a_directory(&zipArchive, i))
			{
				continue;
			}

			std::filesystem::path extractPath = dstDir / fileState.m_filename;
			std::filesystem::create_directories(extractPath.parent_path());

			if (!mz_zip_reader_extract_to_file(&zipArchive, i, extractPath.string().c_str(), 0))
			{
				printf("Unable to extract file %s!\n", extractPath.string().c_str());
			}
		}

		mz_zip_reader_end(&zipArchive);
	}

	void RunSetup(const std::filesystem::path& engineDir)
	{
		if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
		{
			if (FileSystem::GetEnvVariable("VOLT_PATH") != engineDir.string())
			{
				FileSystem::SetEnvVariable("VOLT_PATH", engineDir.string());
			}
		}
		else
		{
			FileSystem::SetEnvVariable("VOLT_PATH", engineDir.string());
		}

		const std::string sandboxLaunchCommand = engineDir.string() + "\\Sandbox.exe %1";

		FileSystem::SetRegistryValue(R"(Software\Classes\.vtproj)", "Volt.Sandbox");
		FileSystem::SetRegistryValue(R"(Software\Classes\.vtproj\Content Type)", "text/plain");
		FileSystem::SetRegistryValue(R"(Software\Classes\.vtproj\PerceivedType)", "text");
		FileSystem::SetRegistryValue(R"(Software\Classes\Volt.Sandbox)", "Volt Sandbox");
		FileSystem::SetRegistryValue(R"(Software\Classes\Volt.Sandbox\Shell\Open\Command)", sandboxLaunchCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
}

inline static ImVec4 ToNormalizedRGB(float r, float g, float b, float a = 255.f)
{
	return ImVec4{ r / 255.f, g / 255.f, b / 255.f, a / 255.f };
}

void LauncherLayer::OnAttach()
{
	if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
	{
		const std::filesystem::path engineDir = FileSystem::GetEnvVariable("VOLT_PATH");
		const bool hasSandboxExe = std::filesystem::exists(engineDir / "Sandbox.exe");

		if (hasSandboxExe)
		{
			m_data.engineInfo.engineDirectory = engineDir;
		}
	}

	g_isRunning = true;
}

void LauncherLayer::OnDetach()
{
	g_isRunning = false;
}

void LauncherLayer::OnUIRender()
{
	const ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;

	if (ImGui::BeginTable("mainTable", 2, flags))
	{
		constexpr float TABS_WIDTH = 300.f;

		ImGui::TableSetupColumn("Column1", 0, TABS_WIDTH);
		ImGui::TableSetupColumn("Column2", 0, ImGui::GetContentRegionAvail().x - TABS_WIDTH);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		UI_DrawTabsChild();

		ImGui::TableNextColumn();

		UI_DrawContentChild();

		ImGui::EndTable();
	}

	UI_DrawAboutModal();
	UI_DrawNewProjectModal();
}

void LauncherLayer::UI_DrawAboutModal()
{
	if (!m_aboutModalOpen)
		return;

	ImGui::OpenPopup("About");
	m_aboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	if (m_aboutModalOpen)
	{
		auto image = Walnut::Application::Get().GetApplicationIcon();
		ImGui::Image(image->GetDescriptorSet(), { 48, 48 });

		ImGui::SameLine();
		Walnut::UI::ShiftCursorX(20.0f);

		ImGui::BeginGroup();
		ImGui::Text("Walnut application framework");
		ImGui::Text("by Studio Cherno.");
		ImGui::EndGroup();

		if (Walnut::UI::ButtonCentered("Close"))
		{
			m_aboutModalOpen = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LauncherLayer::UI_DrawNewProjectModal()
{
	if (!m_newProjectModalOpen)
	{
		return;
	}

	ImGui::OpenPopup("New Project");

	m_newProjectModalOpen = ImGui::BeginPopupModal("New Project", nullptr);
	if (m_newProjectModalOpen)
	{
		UI::PushID();
		if (UI::BeginProperties("Testing"))
		{
			UI::PropertyDirectory("Location", m_newProjectData.targetDir);
			UI::Property("Name", m_newProjectData.projectName);

			UI::EndProperties();
		}
		UI::PopID();

		if (ImGui::Button("Cancel"))
		{
			m_newProjectModalOpen = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Create"))
		{
			CreateNewProject(m_newProjectData);
			m_newProjectModalOpen = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void LauncherLayer::UI_DrawTabsChild()
{
	if (ImGui::BeginChild("Tabs"))
	{
		{
			Walnut::UI::ShiftCursorY(10.f);

			Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ToNormalizedRGB(14.f, 134.f, 225.f), ImGuiCol_ButtonActive, ToNormalizedRGB(0.f, 80.f, 160.f) };
			Walnut::UI::ScopedStyle frameRounding{ ImGuiStyleVar_FrameRounding, 0.f };
			Walnut::UI::ScopedStyle itemSpacing{ ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f } };
			Walnut::UI::ScopedStyle borderSize{ ImGuiStyleVar_FrameBorderSize, 0.f };

			if (ImGui::Button("Projects", ImVec2{ ImGui::GetContentRegionAvail().x, 50.f }))
			{
				m_currentTab = Tab::Projects;
			}

			if (ImGui::Button("Engines", ImVec2{ ImGui::GetContentRegionAvail().x, 50.f }))
			{
				m_currentTab = Tab::Engines;
			}
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawContentChild()
{
	if (ImGui::BeginChild("Content"))
	{
		if (m_currentTab == Tab::Projects)
		{
			UI_DrawProjectsContent();
		}
		else if (m_currentTab == Tab::Engines)
		{
			UI_DrawEnginesContent();
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawProjectsContent()
{
	Walnut::UI::ShiftCursor(40.f, 50.f);

	{
		Walnut::UI::ScopedFont font{ Walnut::Application::GetFont("BoldHeader") };
		ImGui::TextUnformatted("Projects");
	}

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.f);

	if (ImGui::Button("Locate"))
	{
	}

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{
		ShowCreateNewProjectModal();
	}

	if (ImGui::BeginChild("ProjectsChild", ImGui::GetContentRegionAvail()))
	{
		for (const auto& project : m_data.projects)
		{
			if (ImGui::BeginChild(project.name.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 50.f }))
			{
				ImGui::TextUnformatted(project.name.c_str());

				ImGui::EndChild();
			}
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::UI_DrawEnginesContent()
{
	Walnut::UI::ShiftCursor(40.f, 50.f);

	{
		Walnut::UI::ScopedFont font{ Walnut::Application::GetFont("BoldHeader") };
		ImGui::TextUnformatted("Engine");
	}

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 200.f);

	if (ImGui::Button("Locate"))
	{
	}

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{
		const std::string url = "https://github.com/ChunkTreasure1/Volt/releases/download/v0.1.0/Volt.zip";
		g_installProgress = 0.f;
		m_isInstalling = true;

		m_downloadFuture = std::async(std::launch::async, [&, url]()
		{
			TCHAR pf[MAX_PATH];
			SHGetSpecialFolderPath(0, pf, CSIDL_LOCAL_APPDATA, FALSE);

			const std::filesystem::path targetDir = std::filesystem::path(pf) / "Programs" / "Volt" / "Engine";

			Utility::DownloadFile(url, "Volt.zip", [](void*, double t, double d)
			{
				printf("Progress: %f", float(d / t));

				return 0;
			});
		
			Utility::UnzipFile("Volt.zip", targetDir);
			std::filesystem::remove("Volt.zip");

			Utility::RunSetup(targetDir / "Volt");

			m_data.engineInfo.engineDirectory = targetDir / "Volt";
			m_isInstalling = false;
		});
	}

	if (ImGui::BeginChild("EngineChild", ImGui::GetContentRegionAvail()))
	{
		Walnut::UI::ShiftCursorX(10.f);

		if (m_isInstalling)
		{
			ImGui::Text("Installing...");

			Walnut::UI::ShiftCursorX(10.f);
			ImGui::ProgressBar(g_installProgress, ImVec2{ 200.f, 20.f });
		}
		else
		{
			if (!m_data.engineInfo.IsValid())
			{
				ImGui::TextUnformatted("No engine has been found!");
			}
		}

		ImGui::EndChild();
	}
}

void LauncherLayer::ShowAboutModal()
{
	m_aboutModalOpen = true;
}

void LauncherLayer::ShowCreateNewProjectModal()
{
	TCHAR pf[MAX_PATH];
	SHGetSpecialFolderPath(0, pf, CSIDL_MYDOCUMENTS, FALSE);

	m_newProjectData = {};
	m_newProjectData.targetDir = std::filesystem::path(pf) / "Volt Projects";
	m_newProjectData.projectName = "My Project";

	m_newProjectModalOpen = true;
}

void LauncherLayer::CreateNewProject(const CreateProjectData& newData)
{
	if (!m_data.engineInfo.IsValid())
	{
		return;
	}

	const std::filesystem::path projectTemplatePath = m_data.engineInfo.engineDirectory / "Templates" / "Project";
	const std::filesystem::path targetDir = newData.targetDir / newData.projectName;

	if (std::filesystem::exists(targetDir))
	{
		return;
	}

	std::filesystem::create_directories(targetDir);
	std::filesystem::copy(projectTemplatePath, targetDir, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

	auto& newProject = m_data.projects.emplace_back();
	newProject.name = newData.projectName;
	newProject.path = targetDir;
}