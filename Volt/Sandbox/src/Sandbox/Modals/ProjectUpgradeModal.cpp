#include "sbpch.h"
#include "ProjectUpgradeModal.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/FileIO/YAMLStreamReader.h>

#include <imgui.h>

ProjectUpgradeModal::ProjectUpgradeModal(const std::string& strId)
	: Modal(strId)
{
}

void ProjectUpgradeModal::DrawModalContent()
{
	const std::string engineVersion = Volt::Application::Get().GetInfo().version.ToString();
	const std::string projectVersion = Volt::ProjectManager::GetProject().engineVersion.ToString();

	ImGui::TextUnformatted("The loaded project is not compatible with this version of the engine!");

	if (projectVersion.empty())
	{
		ImGui::Text("Would you like to upgrade the project to version %s?", engineVersion.c_str());
	}
	else
	{
		ImGui::Text("Would you like to upgrade the project from version %s to version %s?", projectVersion.c_str(), engineVersion.c_str());
	}

	if (ImGui::Button("Yes"))
	{
		UpgradeCurrentProject();

		ImGui::CloseCurrentPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("No"))
	{
		ImGui::CloseCurrentPopup();
	}
}

void ProjectUpgradeModal::OnOpen()
{
}

void ProjectUpgradeModal::OnClose()
{
}

void ProjectUpgradeModal::UpgradeCurrentProject()
{
	const Volt::Version engineVersion = Volt::Application::Get().GetInfo().version;
	const Volt::Version projectVersion = Volt::ProjectManager::GetProject().engineVersion;

	if (projectVersion.GetMinor() < 1 && projectVersion.GetMajor() == 0)
	{
		ConvertMetaFilesFromV0();
	}

	Volt::ProjectManager::OnProjectUpgraded();
	Volt::ProjectManager::SerializeProject();
}

void ProjectUpgradeModal::ConvertMetaFilesFromV0()
{
	auto& project = Volt::ProjectManager::GetProject();

	const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

	std::vector<std::filesystem::path> metaFilesToConvert;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath))
	{
		if (entry.path().extension().string() != ".vtmeta")
		{
			continue;
		}

		metaFilesToConvert.emplace_back(entry.path());
	}

	struct AssetDescriptor
	{
		Volt::AssetHandle handle;
		std::filesystem::path assetFilePath;
	};

	std::vector<AssetDescriptor> assetDescriptors;

	for (const auto& metaPath : metaFilesToConvert)
	{
		const auto [assetPath, handle] = DeserializeV0MetaFile(metaPath);
		assetDescriptors.emplace_back(handle, assetPath);

		if (std::filesystem::exists(metaPath))
		{
			std::filesystem::remove(metaPath);
		}
	}

	for (const auto& descriptor : assetDescriptors)
	{
		Volt::AssetManager::Get().AddAssetToRegistry(descriptor.assetFilePath, descriptor.handle);
	}
}

std::pair<std::filesystem::path, Volt::AssetHandle> ProjectUpgradeModal::DeserializeV0MetaFile(const std::filesystem::path& metaPath)
{
	Volt::YAMLStreamReader streamReader{};

	if (!streamReader.OpenFile(metaPath))
	{
		return {};
	}

	std::filesystem::path resultAssetFilePath;
	Volt::AssetHandle resultAssetHandle = 0;

	// Is new
	if (streamReader.HasKey("Metadata"))
	{
		streamReader.EnterScope("Metadata");

		resultAssetFilePath = streamReader.ReadKey("filePath", std::string(""));
		resultAssetHandle = streamReader.ReadKey("assetHandle", 0u);

		streamReader.ExitScope();
	}
	else if (streamReader.HasKey("Handle"))
	{
		resultAssetHandle = streamReader.ReadKey("Handle", uint64_t(0));
		resultAssetFilePath = streamReader.ReadKey("Path", std::string(""));
	}

	return { resultAssetFilePath, resultAssetHandle };
}
