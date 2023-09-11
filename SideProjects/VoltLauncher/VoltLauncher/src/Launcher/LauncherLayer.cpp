#define CURL_STATICLIB

#include "LauncherLayer.h"

#include "FileSystem.h"
#include "UIUtility.h"
#include "SerializationMacros.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include "../vendor/miniz.h"

#include <Windows.h>
#include <Shlobj.h>

#include <curl/curl.h>
#include <yaml-cpp/yaml.h>

#include <fstream>

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
	m_editIcon = std::make_shared<Walnut::Image>("Icons/icon_edit.png");
	m_playIcon = std::make_shared<Walnut::Image>("Icons/icon_play.png");
	m_dotsIcon = std::make_shared<Walnut::Image>("Icons/icon_dots.png");

	DeserializeData();

	if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
	{
		const std::filesystem::path engineDir = FileSystem::GetEnvVariable("VOLT_PATH");
		const bool hasSandboxExe = std::filesystem::exists(engineDir / "Sandbox.exe");

		if (hasSandboxExe)
		{
			m_data.engineInfo.engineDirectory = engineDir;
		}
	}
	else if (m_data.engineInfo.IsValid())
	{
		m_data.engineInfo.needsRepair = true;
	}

	g_isRunning = true;
}

void LauncherLayer::OnDetach()
{
	g_isRunning = false;

	SerializeData();

	m_playIcon = nullptr;
	m_editIcon = nullptr;
	m_dotsIcon = nullptr;
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
		auto projectFilePath = FileSystem::OpenFileDialogue({ {"Volt Project", "vtproj"} });
		if (std::filesystem::exists(projectFilePath) && projectFilePath.extension() == ".vtproj")
		{
			if (!IsProjectRegistered(projectFilePath))
			{
				m_data.projects.emplace_back() = ReadProjectInfo(projectFilePath);
				SerializeData();
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Add"))
	{
		ShowCreateNewProjectModal();
	}

	Walnut::UI::ShiftCursor(40.f, 20.f);

	if (ImGui::BeginChild("ProjectsChild", ImGui::GetContentRegionAvail()))
	{
		int32_t removeIndex = -1;
		int32_t index = 0;

		for (const auto& project : m_data.projects)
		{
			UI::PushID();

			const float contentAreaWidth = ImGui::GetContentRegionAvail().x - 40.f;
			const float contentAreaHeight = 70.f;

			if (ImGui::BeginChild(project.name.c_str(), ImVec2{ contentAreaWidth, contentAreaHeight }, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				Walnut::UI::ShiftCursorY(15.f);
				ImGui::TextUnformatted(project.name.c_str());

				ImGui::SameLine(contentAreaWidth - 170);

				Walnut::UI::ShiftCursorY(-15.f);

				if (ImGui::ImageButton(m_playIcon->GetDescriptorSet(), { 32, 32 }))
				{
					if (m_data.engineInfo.IsValid() && !m_data.engineInfo.needsRepair)
					{
						FileSystem::StartProcess(m_data.engineInfo.engineDirectory / "Launcher.exe", project.path.wstring());
					}
				}

				ImGui::SameLine();

				if (ImGui::ImageButton(m_editIcon->GetDescriptorSet(), { 32, 32 }))
				{
					FileSystem::StartProcess(m_data.engineInfo.engineDirectory / "Sandbox.exe", project.path.wstring());
				}

				ImGui::SameLine();

				ImGui::ImageButton(m_dotsIcon->GetDescriptorSet(), { 32, 32 });
				if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
				{
					Walnut::UI::ShiftCursorY(10.f);
					{
						Walnut::UI::ScopedStyle frameRounding{ ImGuiStyleVar_FrameRounding, 0.f };
						Walnut::UI::ScopedStyle itemSpacing{ ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f } };
						Walnut::UI::ScopedStyle borderSize{ ImGuiStyleVar_FrameBorderSize, 0.f };

						{
							Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ToNormalizedRGB(14.f, 134.f, 225.f), ImGuiCol_ButtonActive, ToNormalizedRGB(0.f, 80.f, 160.f) };

							if (ImGui::Button("Reveal in explorer", ImVec2{ ImGui::GetContentRegionAvail().x, 30.f }))
							{
								FileSystem::ShowDirectoryInExplorer(project.path.parent_path());
							}

						}

						ImGui::Separator();

						{
							Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.f }, ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.f } };

							if (ImGui::Button("Remove", ImVec2{ ImGui::GetContentRegionAvail().x, 30.f }))
							{
								FileSystem::MoveToRecycleBin(project.path.parent_path());

								removeIndex = index;
							}
						}
					}

					ImGui::EndPopup();
				}

				ImGui::EndChild();
			}

			UI::PopID();
			index++;
		}

		if (removeIndex != -1)
		{
			m_data.projects.erase(m_data.projects.begin() + removeIndex);
			SerializeData();
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

	const bool installed = m_data.engineInfo.IsValid();

	if (ImGui::Button("Locate"))
	{
	}

	if (m_data.engineInfo.needsRepair)
	{
		ImGui::SameLine();

		if (ImGui::Button("Repair"))
		{
			Utility::RunSetup(m_data.engineInfo.engineDirectory);
			m_data.engineInfo.needsRepair = false;
		}
	}

	if (!installed)
	{
		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			const std::string url = "https://github.com/ChunkTreasure1/Volt/releases/download/v0.1.2/Volt.zip";
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
	}

	Walnut::UI::ShiftCursor(40.f, 20.f);

	if ((m_isInstalling || installed) && ImGui::BeginChild("EngineChild", ImGui::GetContentRegionAvail()))
	{
		const float contentAreaWidth = ImGui::GetContentRegionAvail().x - 40.f;
		const float contentAreaHeight = 70.f;

		if (ImGui::BeginChild("Engine", ImVec2{ contentAreaWidth, contentAreaHeight }, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			Walnut::UI::ShiftCursorY(10.f);
			ImGui::TextUnformatted("Volt");

			if (m_isInstalling)
			{
				ImGui::SameLine();

				Walnut::UI::ShiftCursorY(-5.f);
				ImGui::ProgressBar(g_installProgress, ImVec2{ contentAreaWidth - 200.f, 30.f });
			}

			ImGui::SameLine(contentAreaWidth - 80.f);

			Walnut::UI::ShiftCursorY(-10.f);

			ImGui::ImageButton(m_dotsIcon->GetDescriptorSet(), { 32.f, 32.f });
			if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
			{
				Walnut::UI::ShiftCursorY(10.f);
				{
					Walnut::UI::ScopedStyle frameRounding{ ImGuiStyleVar_FrameRounding, 0.f };
					Walnut::UI::ScopedStyle itemSpacing{ ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f } };
					Walnut::UI::ScopedStyle borderSize{ ImGuiStyleVar_FrameBorderSize, 0.f };

					{
						Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ToNormalizedRGB(14.f, 134.f, 225.f), ImGuiCol_ButtonActive, ToNormalizedRGB(0.f, 80.f, 160.f) };

						if (ImGui::Button("Reveal in explorer", ImVec2{ ImGui::GetContentRegionAvail().x, 30.f }))
						{
							FileSystem::ShowDirectoryInExplorer(m_data.engineInfo.engineDirectory);
						}
					}

					ImGui::Separator();

					{
						Walnut::UI::ScopedColorStack buttonColor{ ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f }, ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.f }, ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.f } };

						if (ImGui::Button("Remove", ImVec2{ ImGui::GetContentRegionAvail().x, 30.f }))
						{
							FileSystem::MoveToRecycleBin(m_data.engineInfo.engineDirectory);
							m_data.engineInfo.engineDirectory = "";

							SerializeData();
						}
					}
				}

				ImGui::EndPopup();
			}

			if (m_isInstalling)
			{
				Walnut::UI::ShiftCursorY(-10.f);
			}
			else
			{
				Walnut::UI::ShiftCursorY(-15.f);
			}

			ImGui::Text("Install Directory: %s", m_data.engineInfo.engineDirectory.string().c_str());

			ImGui::EndChild();
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
	newProject.path = targetDir / "Project.vtproj";

	WriteProjectInfo(newProject.path, newProject);
	SerializeData();
}

void LauncherLayer::SerializeData()
{
	YAML::Emitter out{};
	out << YAML::BeginMap;
	out << YAML::Key << "Data" << YAML::Value;
	{
		out << YAML::BeginMap;
		VT_SERIALIZE_PROPERTY(engineDirectory, m_data.engineInfo.engineDirectory.string(), out);

		out << YAML::Key << "Projects" << YAML::BeginSeq;
		for (const auto& project : m_data.projects)
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(name, project.name, out);
			VT_SERIALIZE_PROPERTY(path, project.path.string(), out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	out << YAML::EndMap;

	std::ofstream fout{ "Data.yaml" };
	fout << out.c_str();
	fout.close();
}

void LauncherLayer::DeserializeData()
{
	std::ifstream file{ "Data.yaml" };
	if (!file.is_open())
	{
		return;
	}

	std::stringstream sstream{};
	sstream << file.rdbuf();
	file.close();

	YAML::Node root{};

	try
	{
		root = YAML::Load(sstream.str());
	}
	catch (std::exception&)
	{
		return;
	}

	YAML::Node dataRoot = root["Data"];

	m_data.engineInfo.engineDirectory = dataRoot["engineDirectory"].as<std::string>();

	for (const auto& projectNode : dataRoot["Projects"])
	{
		auto& project = m_data.projects.emplace_back();

		project.name = projectNode["name"].as<std::string>();
		project.path = projectNode["path"].as<std::string>();
	}
}

Project LauncherLayer::ReadProjectInfo(const std::filesystem::path& projectFilePath)
{
	Project result{};
	result.path = projectFilePath;

	std::ifstream file{ projectFilePath };
	if (!file.is_open())
	{
		return { "Null" };
	}

	std::stringstream sstream{};
	sstream << file.rdbuf();
	file.close();

	YAML::Node root{};

	try
	{
		root = YAML::Load(sstream.str());
	}
	catch (std::exception&)
	{
		return { "Null" };
	}

	if (!root["Project"])
	{
		return { "Null" };
	}

	if (!root["Project"]["Name"])
	{
		return { "Null" };
	}

	result.name = root["Project"]["Name"].as<std::string>();

	return result;
}

void LauncherLayer::WriteProjectInfo(const std::filesystem::path& projectFilePath, const Project& project)
{
	YAML::Node projectFile = YAML::LoadFile(projectFilePath.string());

	projectFile["Project"]["Name"] = project.name;

	std::ofstream fout{ projectFilePath };
	fout << projectFile;

	fout.close();
}

const bool LauncherLayer::IsProjectRegistered(const std::filesystem::path& projectFilePath)
{
	for (const auto& project : m_data.projects)
	{
		if (project.path == projectFilePath)
		{
			return true;
		}
	}

	return false;
}
