#include "sbpch.h"
#include "GameBuilder.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Shader/Shader.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Utility/YAMLSerializationHelpers.h>
#include <Volt/Utility/SerializationMacros.h>

#include <yaml-cpp/yaml.h>

namespace Utility
{
	inline bool ShouldSkipDLL(const std::filesystem::path& path)
	{
		const std::vector<std::string> skipItems =
		{
			"dpp",
			"libcrypto-1_1-x64",
			"libsodium",
			"libssl-1_1-x64",
			"opus",
			"zlib1",
			"fmodL",
			"fmodstudioL"
		};

		return std::find(skipItems.begin(), skipItems.end(), path.stem().string()) != skipItems.end();
	}

	inline bool ShouldSkipAsset(const std::filesystem::path& path)
	{
		const std::vector<Volt::AssetType> skipItems =
		{
			Volt::AssetType::None,
			Volt::AssetType::MeshSource,
			Volt::AssetType::ShaderSource
		};

		const std::vector<std::string> skipExtensions =
		{
			".png",
			".jpeg",
			".jpg",
			".tga"
		};

		const auto assetType = Volt::AssetManager::Get().GetAssetTypeFromPath(path);

		bool result = std::find(skipItems.begin(), skipItems.end(), assetType) != skipItems.end();
		result |= std::find_if(skipExtensions.begin(), skipExtensions.end(), [&](const std::string ext) { return path.extension().string() == ext; }) != skipExtensions.end();

		return result;
	}

	inline bool IsTexturePow2(const std::filesystem::path& path)
	{
		if (path.extension().string() == ".hdr")
		{
			return true;
		}

		const Volt::AssetHandle handle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);

		const bool wasLoaded = Volt::AssetManager::Get().IsLoaded(handle);

		Ref<Volt::Texture2D> texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(handle);

		const uint32_t width = texture->GetWidth();
		const uint32_t height = texture->GetHeight();

		const bool hPow2 = height && ((height & (height - 1)) == 0);
		const bool wPow2 = width && ((width & (width - 1)) == 0);

		if (!wasLoaded)
		{
			Volt::AssetManager::Get().Unload(handle);
		}

		return hPow2 && wPow2;
	}

	inline bool IsEntity(const std::filesystem::path& path)
	{
		return path.extension().string() == ".ent";
	}
}

void GameBuilder::BuildGame(const BuildInfo& buildInfo)
{
	if (myIsBuilding)
	{
		UI::Notify(NotificationType::Error, "Failed to start build!", "A build is already running!");
		return;
	}

	if (!std::filesystem::exists(buildInfo.buildDirectory))
	{
		UI::Notify(NotificationType::Error, "Build failed!", "Build directory does not exist!");
		return;
	}

	// Recompile all shaders
	{
		for (const auto& [name, shader] : Volt::ShaderRegistry::GetAllShaders())
		{
			shader->Reload(true);
		}
	}

	myIsBuilding = true;
	myCurrentFileNumber = 0;
	myRelevantFileCount = GetRelevantFileCount(buildInfo);

	myBuildStartTime = std::chrono::high_resolution_clock::now();

	std::thread thread{ Thread_BuildGame, std::reference_wrapper(buildInfo) };
	thread.detach();
}

float GameBuilder::GetBuildProgress()
{
	return (float)myCurrentFileNumber / (float)myRelevantFileCount;
}

std::string GameBuilder::GetCurrentFile()
{
	std::scoped_lock lock(myMutex);
	return myCurrentFile;
}

void GameBuilder::Thread_BuildGame(const BuildInfo& buildInfo)
{
	FileSystem::CopyFileTo("Launcher.exe", buildInfo.buildDirectory);

	for (const auto& file : std::filesystem::directory_iterator("."))
	{
		if (file.path().extension().string() == ".dll" && !Utility::ShouldSkipDLL(file.path()))
		{
			myCurrentFileNumber++;

			{
				std::scoped_lock lock(myMutex);
				myCurrentFile = file.path().stem().string();
			}

			FileSystem::CopyFileTo(file.path(), buildInfo.buildDirectory);
		}
	}

	// Copy Engine folder
	{
		const auto enginePath = buildInfo.buildDirectory / "Engine";
		FileSystem::CreateFolder(enginePath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetEnginePath()))
		{
			if (!file.is_directory())
			{
				const auto relPath = std::filesystem::relative(file.path(), FileSystem::GetEnginePath()).parent_path();

				if (!FileSystem::Exists(enginePath / relPath))
				{
					FileSystem::CreateFolder(enginePath / relPath);
				}

				{
					std::scoped_lock lock(myMutex);
					myCurrentFile = file.path().stem().string();
				}

				myCurrentFileNumber++;
				FileSystem::CopyFileTo(file.path(), enginePath / relPath);
			}
		}

		// Remove HLSL path
		FileSystem::Remove(enginePath / "Shaders/HLSL/");
	}

	// Copy Assets folder
	{
		const auto assetsPath = buildInfo.buildDirectory / "Assets";
		FileSystem::CreateFolder(assetsPath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetAssetsPath()))
		{
			if (!file.is_directory())
			{
				const auto assetType = Volt::AssetManager::Get().GetAssetTypeFromPath(file.path());

				if (Utility::ShouldSkipAsset(file.path()) && !Utility::IsEntity(file.path()) && file.path().extension().string() != ".yaml")
				{
					continue;
				}

				if (assetType == Volt::AssetType::Texture)
				{
					if (!Utility::IsTexturePow2(file.path()))
					{
						continue;
					}
				}

				const auto relPath = std::filesystem::relative(file.path(), FileSystem::GetAssetsPath()).parent_path();

				if (!FileSystem::Exists(assetsPath / relPath))
				{
					FileSystem::CreateFolder(assetsPath / relPath);
				}

				{
					std::scoped_lock lock(myMutex);
					myCurrentFile = file.path().stem().string();
				}

				myCurrentFileNumber++;
				FileSystem::CopyFileTo(file.path(), assetsPath / relPath);
			}
		}

		// Copy asset registry
		myCurrentFileNumber++;
		FileSystem::CopyFileTo("Assets/AssetRegistry.vtreg", assetsPath);
	}

	// Generate scene dependencies
	{
		for (const auto& handle : buildInfo.sceneHandles)
		{
			const std::filesystem::path path = Volt::AssetManager::Get().GetPathFromAssetHandle(handle);

			const auto targetScenePath = buildInfo.buildDirectory / path;

			const Volt::AssetHandle sceneHandle = Volt::AssetManager::Get().GetAssetHandleFromPath(path);
			const bool isLoaded = Volt::AssetManager::Get().IsLoaded(sceneHandle);

			Ref<Volt::Scene> scene = Volt::AssetManager::GetAsset<Volt::Scene>(sceneHandle);
			std::set<Volt::AssetHandle> sceneDependencies;

			auto& registry = scene->GetRegistry();
			for (const auto& ent : registry.GetAllEntities())
			{
				for (const auto& [guid, pool] : registry.GetPools())
				{
					if (pool->HasComponent(ent))
					{
						const auto& compInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
						for (const auto& prop : compInfo.properties)
						{
							if (prop.type == Wire::ComponentRegistry::PropertyType::AssetHandle)
							{
								uint8_t* data = (uint8_t*)registry.GetComponentPtr(guid, ent);

								Volt::AssetHandle assetHandle = *(Volt::AssetHandle*)&data[prop.offset];
								if (assetHandle != Volt::Asset::Null() && Volt::AssetManager::Get().ExistsInRegistry(assetHandle))
								{
									sceneDependencies.emplace(assetHandle);
								}
							}
						}
					}
				}
			}

			// Save dependency list
			{
				YAML::Emitter out;
				out << YAML::BeginMap;

				out << YAML::Key << "Dependencies" << YAML::BeginSeq;
				for (const auto& dep : sceneDependencies)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Handle" << YAML::Value << dep;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;

				const std::filesystem::path dependeciesPath = targetScenePath.parent_path() / "Dependencies.vtdep";

				std::ofstream fout(dependeciesPath);
				fout << out.c_str();
				fout.close();
			}
		}
	}

	myIsBuilding = false;
}

uint32_t GameBuilder::GetRelevantFileCount(const BuildInfo& buildInfo)
{
	uint32_t result = 1; // Exe

	for (const auto& file : std::filesystem::directory_iterator("."))
	{
		if (file.path().extension().string() == ".dll" && !Utility::ShouldSkipDLL(file.path()))
		{
			result++;
		}
	}

	// Engine folder
	{
		const auto enginePath = buildInfo.buildDirectory / "Engine";
		FileSystem::CreateFolder(enginePath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetEnginePath()))
		{
			if (!file.is_directory() && !file.path().string().contains("HLSL"))
			{
				result++;
			}
		}
	}

	// Assets folder
	{
		const auto assetsPath = buildInfo.buildDirectory / "Assets";
		FileSystem::CreateFolder(assetsPath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetAssetsPath()))
		{
			if (!file.is_directory())
			{
				if (Utility::ShouldSkipAsset(file.path()) && !Utility::IsEntity(file.path()))
				{
					continue;
				}

				result++;
			}
		}

		// Asset registry
		result++;
	}

	// Scene dependencies
	{
		result += (uint32_t)buildInfo.sceneHandles.size();
	}

	return result;
}
