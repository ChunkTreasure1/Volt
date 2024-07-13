#include "sbpch.h"
#include "AssetDirectoryProcessor.h"

#include "Sandbox/Window/AssetBrowser/DirectoryItem.h"
#include "Sandbox/Window/AssetBrowser/AssetItem.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Asset/AssetManager.h>

AssetDirectoryProcessor::AssetDirectoryProcessor(Weak<AssetBrowser::SelectionManager> selectionManager, Volt::AssetType assetMask)
	: m_selectionManager(selectionManager), m_assetMask(assetMask)
{}

Ref<AssetBrowser::DirectoryItem> AssetDirectoryProcessor::ProcessDirectories(const std::filesystem::path& path, MeshImportData& meshImportData, AssetData& meshToImportData)
{
	struct AssetEntryData
	{
		std::filesystem::path path;
		bool isDirectory;
	};

	std::vector<AssetEntryData> assetEntries;

	std::vector<std::filesystem::path> pathsToProcess;
	pathsToProcess.emplace_back(path);

	while (!pathsToProcess.empty())
	{
		auto currentPath = pathsToProcess.back();
		pathsToProcess.pop_back();

		for (const auto& entry : std::filesystem::directory_iterator(currentPath))
		{
			auto& assetEntry = assetEntries.emplace_back();
			assetEntry.path = entry.path();
			assetEntry.isDirectory = entry.is_directory();

			if (assetEntry.isDirectory)
			{
				pathsToProcess.emplace_back(assetEntry.path);
			}
		}
	}

	auto relStartPath = Volt::AssetManager::GetRelativePath(path);
	Ref<AssetBrowser::DirectoryItem> resultItem = CreateRef<AssetBrowser::DirectoryItem>(m_selectionManager.Get(), relStartPath);
	std::unordered_map<std::filesystem::path, Ref<AssetBrowser::DirectoryItem>> directoryItems;

	directoryItems[relStartPath] = resultItem;

	for (const auto& entry : assetEntries)
	{
		if (entry.isDirectory)
		{
			auto relPath = Volt::AssetManager::GetRelativePath(entry.path);
			Ref<AssetBrowser::DirectoryItem> dirData = CreateRef<AssetBrowser::DirectoryItem>(m_selectionManager.Get(), relPath);
			directoryItems[relPath] = dirData;
			const auto parentPath = Volt::AssetManager::GetRelativePath(entry.path.parent_path());
			directoryItems[parentPath]->subDirectories.emplace_back(dirData);
			dirData->parentDirectory = directoryItems[parentPath].get();
		}
		else
		{
			auto type = Volt::AssetManager::GetAssetTypeFromExtension(entry.path.extension().string());
			if (type == Volt::AssetType::None)
			{
				type = Volt::AssetManager::GetAssetTypeFromPath(entry.path);
			}

			const auto filename = entry.path.filename().string();

			if (type != Volt::AssetType::None && !Utility::StringContains(filename, ".vtthumb.png"))
			{
				if (m_assetMask == Volt::AssetType::None || (m_assetMask & type) != Volt::AssetType::None)
				{
					auto relPath = Volt::AssetManager::GetRelativePath(entry.path);
					Ref<AssetBrowser::AssetItem> assetItem = CreateRef<AssetBrowser::AssetItem>(m_selectionManager.Get(), relPath, meshImportData, meshToImportData);
					const auto parentPath = Volt::AssetManager::GetRelativePath(entry.path.parent_path());
					directoryItems[parentPath]->assets.emplace_back(assetItem);
				}
			}
		}
	}

	for (const auto& [dirPath, dirData] : directoryItems)
	{
		std::sort(dirData->subDirectories.begin(), dirData->subDirectories.end(), [](const Ref<AssetBrowser::DirectoryItem>& a, const Ref<AssetBrowser::DirectoryItem>& b) { return a->path.string() < b->path.string(); });
		std::sort(dirData->assets.begin(), dirData->assets.end(), [](const Ref<AssetBrowser::AssetItem>& a, const Ref<AssetBrowser::AssetItem>& b) { return a->path.stem().string() < b->path.stem().string(); });
	}

	return resultItem;
}
