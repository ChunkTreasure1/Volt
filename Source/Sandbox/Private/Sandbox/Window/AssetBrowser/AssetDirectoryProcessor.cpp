#include "sbpch.h"
#include "Sandbox/Window/AssetBrowser/AssetDirectoryProcessor.h"

#include "Sandbox/Window/AssetBrowser/DirectoryItem.h"
#include "Sandbox/Window/AssetBrowser/AssetItem.h"
#include "Sandbox/Window/AssetBrowser/AssetCommon.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <AssetSystem/AssetManager.h>

AssetDirectoryProcessor::AssetDirectoryProcessor(Weak<AssetBrowser::SelectionManager> selectionManager, std::set<AssetType> assetMask)
	: m_selectionManager(selectionManager), m_assetMask(assetMask)
{}

Ref<AssetBrowser::DirectoryItem> AssetDirectoryProcessor::ProcessDirectories(const std::filesystem::path& path, MeshImportData& meshImportData, AssetData& meshToImportData)
{
	struct AssetEntryData
	{
		std::filesystem::path path;
		bool isDirectory;
	};

	Vector<AssetEntryData> assetEntries;

	Vector<std::filesystem::path> pathsToProcess;
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
			AssetType type = Volt::AssetManager::GetAssetTypeFromPath(entry.path);
			if (type == AssetTypes::None)
			{
				type = GetAssetTypeRegistry().GetTypeFromExtension(entry.path.extension().string());
			}

			const auto filename = entry.path.filename().string();

			if (type != AssetTypes::None && !Utility::StringContains(filename, ".vtthumb.png"))
			{
				if (m_assetMask.empty() || m_assetMask.contains(type))
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
