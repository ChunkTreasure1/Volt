#include "sbpch.h"
#include "V011Convert.h"

#include <Volt/Project/ProjectManager.h>
#include <Volt/Asset/Asset.h>
#include <Volt/Asset/AssetManager.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>

namespace V011
{
	inline std::pair<std::filesystem::path, Volt::AssetHandle> DeserializeV0MetaFile(const std::filesystem::path& metaPath)
	{
		YAMLFileStreamReader streamReader{};

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

			resultAssetFilePath = streamReader.ReadAtKey("filePath", std::string(""));
			resultAssetHandle = streamReader.ReadAtKey("assetHandle", uint64_t(0u));

			streamReader.ExitScope();
		}
		else if (streamReader.HasKey("Handle"))
		{
			resultAssetHandle = streamReader.ReadAtKey("Handle", uint64_t(0));
			resultAssetFilePath = streamReader.ReadAtKey("Path", std::string(""));
		}

		return { resultAssetFilePath, resultAssetHandle };
	}

	inline void ConvertMetaFilesToV011()
	{
		auto& project = Volt::ProjectManager::GetProject();

		const std::filesystem::path assetsPath = project.projectDirectory / project.assetsDirectory;

		Vector<std::filesystem::path> metaFilesToConvert;

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

		Vector<AssetDescriptor> assetDescriptors;

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
