#include "sbpch.h"
#include "V011Convert.h"

#include <Volt/Project/ProjectManager.h>
#include <Volt/Asset/Asset.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/FileIO/YAMLFileStreamReader.h>

namespace V011
{
	inline std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath)
	{
		Volt::YAMLFileStreamReader streamReader{};

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
			resultAssetHandle = streamReader.ReadKey("assetHandle", uint64_t(0u));

			streamReader.ExitScope();
		}
		else if (streamReader.HasKey("Handle"))
		{
			resultAssetHandle = streamReader.ReadKey("Handle", uint64_t(0));
			resultAssetFilePath = streamReader.ReadKey("Path", std::string(""));
		}

		return { resultAssetFilePath, resultAssetHandle };
	}

	inline void ConvertMetaFilesToV011()
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
			auto [assetPath, handle] = DeserializeV0MetaFile(metaPath);
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

	void Convert()
	{
		ConvertMetaFilesToV011();
	}
}
