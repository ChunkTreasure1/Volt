#include "sbpch.h"
#include "Sandbox.h"

#include <Volt/Asset/Rendering/ShaderDefinition.h>

#include <RenderCore/Shader/ShaderMap.h>

#include <Volt/Utility/UIUtility.h>

void Sandbox::CreateModifiedWatch()
{
	m_fileWatcher->AddCallback(efsw::Actions::Modified, [&](const auto newPath, const auto oldPath)
	{
		if (newPath.extension().string() == ".nv-gpudmp" || oldPath.extension().string() == ".nv-gpudmp")
		{
			return;
		}

		if (std::filesystem::is_directory(newPath))
		{
			return;
		}

		std::scoped_lock lock(m_fileWatcherMutex);
		m_fileChangeQueue.emplace_back([newPath, oldPath, this]()
		{
			auto assemblyPath = Volt::ProjectManager::GetMonoAssemblyPath();
			if (Utility::StringContains((newPath.parent_path().filename() / newPath.filename()).string(), (assemblyPath.parent_path().filename() / assemblyPath.filename()).string()))
			{
				if (m_sceneState == SceneState::Play)
				{
					Sandbox::Get().OnSceneStop();
				}

				UI::Notify(NotificationType::Success, "C# Assembly Reloaded!", "The C# assembly was reloaded successfully!");
				return;
			}

			AssetType assetType = Volt::AssetManager::GetAssetTypeFromPath(newPath);
			if (assetType == AssetTypes::Mesh ||
				assetType == AssetTypes::Prefab ||
				assetType == AssetTypes::Material ||
				assetType == AssetTypes::Texture)
			{
				Volt::AssetManager::Get().ReloadAsset(Volt::AssetManager::GetRelativePath(newPath));
			}
			else if (assetType == AssetTypes::ShaderSource)
			{
				const auto dependents = Volt::AssetManager::GetAssetsDependentOn(Volt::AssetManager::GetAssetHandleFromFilePath(newPath));

				for (const auto& assetHandle : dependents)
				{
					const auto dependentType = Volt::AssetManager::GetAssetTypeFromHandle(assetHandle);
					if (dependentType != AssetTypes::ShaderDefinition)
					{
						continue;
					}

					Ref<Volt::ShaderDefinition> shaderDef = Volt::AssetManager::GetAsset<Volt::ShaderDefinition>(assetHandle);
					bool succeded = Volt::ShaderMap::ReloadShaderByName(std::string(shaderDef->GetName()));
					if (succeded)
					{
						UI::Notify(NotificationType::Success, "Recompiled shader!", std::format("Shader {0} was successfully recompiled!", shaderDef->GetName()));
					}
					else
					{
						UI::Notify(NotificationType::Error, "Failed to recompile shader!", std::format("Recompilation of shader {0} failed! Check log for more info!", shaderDef->GetName()));
					}
				}
			}
			else if (assetType == AssetTypes::MeshSource)
			{
				/*const auto assets = Volt::AssetManager::GetAllAssetsWithDependency(Volt::AssetManager::Get().GetRelativePath(newPath));
for (const auto& asset : assets)
{
	if (EditorUtils::ReimportSourceMesh(asset))
	{
		UI::Notify(NotificationType::Success, "Re imported mesh!", std::format("Mesh {0} has been reimported!", Volt::AssetManager::GetFilePathFromAssetHandle(asset).string()));
	}
}*/
			}
		});
	});
}

void Sandbox::CreateDeleteWatch()
{
	m_fileWatcher->AddCallback(efsw::Actions::Delete, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(m_fileWatcherMutex);
		m_fileChangeQueue.emplace_back([newPath, oldPath]()
		{
			if (!newPath.has_extension())
			{
				Volt::AssetManager::Get().RemoveFullFolderFromRegistry(Volt::AssetManager::GetRelativePath(newPath));
			}
			else
			{
				AssetType assetType = Volt::AssetManager::GetAssetTypeFromPath(Volt::AssetManager::GetRelativePath(newPath));
				if (assetType != AssetTypes::None)
				{
					if (Volt::AssetManager::ExistsInRegistry(Volt::AssetManager::GetRelativePath(newPath)))
					{
						Volt::AssetManager::Get().RemoveAssetFromRegistry(Volt::AssetManager::GetRelativePath(newPath));
					}
				}
			}
		});
	});
}

void Sandbox::CreateAddWatch()
{
	m_fileWatcher->AddCallback(efsw::Actions::Add, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(m_fileWatcherMutex);
	});
}

void Sandbox::CreateMovedWatch()
{
	m_fileWatcher->AddCallback(efsw::Actions::Moved, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(m_fileWatcherMutex);
		m_fileChangeQueue.emplace_back([newPath, oldPath]()
		{
			if (!newPath.has_extension())
			{
				Volt::AssetManager::Get().MoveFullFolder(oldPath, newPath);
			}
			else
			{
				Volt::AssetManager::Get().MoveAssetInRegistry(oldPath, newPath);
			}
		});
	});
}
