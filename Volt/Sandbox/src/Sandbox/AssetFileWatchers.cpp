#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include "Sandbox/Window/AssetBrowser/AssetBrowserPanel.h"

#include <Volt/Rendering/Renderer.h>

#include <Volt/Project/ProjectManager.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Utility/UIUtility.h>

void Sandbox::CreateModifiedWatch()
{
	myFileWatcher->AddCallback(efsw::Actions::Modified, [&](const auto newPath, const auto oldPath)
	{
		if (newPath.extension().string() == ".nv-gpudmp" || oldPath.extension().string() == ".nv-gpudmp")
		{
			return;
		}

		std::scoped_lock lock(myFileWatcherMutex);
		myFileChangeQueue.emplace_back([newPath, oldPath, this]()
		{
			auto assemblyPath = Volt::ProjectManager::GetMonoAssemblyPath();
			if (Utility::StringContains((newPath.parent_path().filename() / newPath.filename()).string(), (assemblyPath.parent_path().filename() / assemblyPath.filename()).string()))
			{
				if (mySceneState == SceneState::Play)
				{
					Sandbox::Get().OnSceneStop();
				}
				else if (mySceneState == SceneState::Edit)
				{
					myRuntimeScene->ShutdownEngineScripts();
				}

				Volt::MonoScriptEngine::ReloadAssembly();

				if (mySceneState == SceneState::Edit)
				{
					myRuntimeScene->InitializeEngineScripts();
				}

				UI::Notify(NotificationType::Success, "C# Assembly Reloaded!", "The C# assembly was reloaded successfully!");
				return;
			}

			Volt::AssetType assetType = Volt::AssetManager::GetAssetTypeFromPath(newPath);
			switch (assetType)
			{
				case Volt::AssetType::Mesh:
				case Volt::AssetType::Video:
				case Volt::AssetType::Prefab:
				case Volt::AssetType::Material:
				case Volt::AssetType::Texture:
					Volt::AssetManager::Get().ReloadAsset(Volt::AssetManager::GetRelativePath(newPath));
					break;

				case Volt::AssetType::ShaderSource:
				{
					// #TODO_Ivar: Reimplement
					const auto assets = Volt::AssetManager::GetAllAssetsWithDependency(Volt::AssetManager::GetRelativePath(newPath));
					//for (const auto& asset : assets)
					//{
					//	Ref<Volt::Shader> shader = Volt::AssetManager::GetAsset<Volt::Shader>(asset);
					//	if (!shader || !shader->IsValid())
					//	{
					//		continue;
					//	}

					//	if (shader->Reload(true))
					//	{
					//		UI::Notify(NotificationType::Success, "Recompiled shader!", std::format("Shader {0} was successfully recompiled!", shader->GetName()));

					//		Volt::Renderer::ReloadShader(shader);
					//	}
					//	else
					//	{
					//		UI::Notify(NotificationType::Error, "Failed to recompile shader!", std::format("Recompilation of shader {0} failed! Check log for more info!", shader->GetName()));
					//	}
					//}
					break;
				}


				case Volt::AssetType::MeshSource:
				{
					const auto assets = Volt::AssetManager::GetAllAssetsWithDependency(Volt::AssetManager::Get().GetRelativePath(newPath));
					for (const auto& asset : assets)
					{
						if (EditorUtils::ReimportSourceMesh(asset))
						{
							UI::Notify(NotificationType::Success, "Re imported mesh!", std::format("Mesh {0} has been reimported!", Volt::AssetManager::GetFilePathFromAssetHandle(asset).string()));
						}
					}
					break;
				}

				case Volt::AssetType::None:
					break;
				default:
					break;
			}
		});

		myFileChangeQueue.emplace_back([&]()
		{
			auto assetBrowser = EditorLibrary::Get<AssetBrowserPanel>();
			assetBrowser->Reload();
		});
	});
}

void Sandbox::CreateDeleteWatch()
{
	myFileWatcher->AddCallback(efsw::Actions::Delete, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(myFileWatcherMutex);
		myFileChangeQueue.emplace_back([newPath, oldPath]()
		{
			if (!newPath.has_extension())
			{
				Volt::AssetManager::Get().RemoveFullFolderFromRegistry(Volt::AssetManager::GetRelativePath(newPath));
			}
			else
			{
				Volt::AssetType assetType = Volt::AssetManager::GetAssetTypeFromPath(Volt::AssetManager::GetRelativePath(newPath));
				if (assetType != Volt::AssetType::None)
				{
					if (Volt::AssetManager::ExistsInRegistry(Volt::AssetManager::GetRelativePath(newPath)))
					{
						Volt::AssetManager::Get().RemoveAssetFromRegistry(Volt::AssetManager::GetRelativePath(newPath));
					}
				}
			}
		});

		myFileChangeQueue.emplace_back([&]()
		{
			auto assetBrowser = EditorLibrary::Get<AssetBrowserPanel>();
			assetBrowser->Reload();
		});
	});
}

void Sandbox::CreateAddWatch()
{
	myFileWatcher->AddCallback(efsw::Actions::Add, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(myFileWatcherMutex);

		myFileChangeQueue.emplace_back([&]()
		{
			auto assetBrowser = EditorLibrary::Get<AssetBrowserPanel>();
			assetBrowser->Reload();
		});
	});
}

void Sandbox::CreateMovedWatch()
{
	myFileWatcher->AddCallback(efsw::Actions::Moved, [&](const std::filesystem::path newPath, const std::filesystem::path oldPath)
	{
		std::scoped_lock lock(myFileWatcherMutex);
		myFileChangeQueue.emplace_back([newPath, oldPath]()
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

		myFileChangeQueue.emplace_back([&]()
		{
			auto assetBrowser = EditorLibrary::Get<AssetBrowserPanel>();
			assetBrowser->Reload();
		});
	});
}
