#include "sbpch.h"
#include "GameBuilder.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Project/ProjectManager.h>

#include <Volt/Rendering/RenderPipeline/ShaderRegistry.h>

#include <Volt/Scripting/Mono/MonoScriptClass.h>

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
			Volt::AssetType::MonoScript
		};

		const std::vector<std::string> skipExtensions =
		{
			".png",
			".jpeg",
			".jpg",
			".tga"
		};

		const auto assetType = Volt::AssetManager::GetAssetTypeFromPath(path);

		bool result = std::find(skipItems.begin(), skipItems.end(), assetType) != skipItems.end();
		bool skipExtension = std::find_if(skipExtensions.begin(), skipExtensions.end(), [&](const std::string ext) { return path.extension().string() == ext; }) != skipExtensions.end();
		
		const auto filename = path.filename().string();

		if (skipExtension && !Utils::StringContains(filename, "vtthumb"))
		{
			VT_CORE_ERROR("[Build] Asset {0} with extensin {1} was skipped!", path.string(), path.extension().string());
		}

		return (result || skipExtension);
	}

	inline bool ShouldSkipFileType(const std::filesystem::path& path)
	{
		const std::vector<std::string> includeExtensions =
		{
			".yaml",
			".bank",
			".vtmeta",
			".txt",
			".bnk"
		};

		const bool result = std::find(includeExtensions.begin(), includeExtensions.end(), path.extension().string()) == includeExtensions.end();
		return result;
	}

	inline bool IsTexturePow2(const std::filesystem::path& path)
	{
		if (path.extension().string() == ".hdr")
		{
			return true;
		}

		const Volt::AssetHandle handle = Volt::AssetManager::GetAssetHandleFromFilePath(Volt::AssetManager::GetRelativePath(path));

		const bool wasLoaded = Volt::AssetManager::IsLoaded(handle);

		Ref<Volt::Texture2D> texture = Volt::AssetManager::GetAsset<Volt::Texture2D>(handle);
		if (!texture || !texture->IsValid())
		{
			VT_CORE_ERROR("Texture {0} is not valid!", path.string());
			return false;
		}

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
		return path.extension().string() == ".vtlayer" || path.extension().string() == ".entVp";
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
		//for (const auto& [name, shader] : Volt::ShaderRegistry::GetShaderRegistry())
		//{
		//	shader->Reload(true);
		//}
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

	// Copy DLL files
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
		FileSystem::CreateDirectory(enginePath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetEnginePath()))
		{
			if (!file.is_directory() && file.path().extension().string() != ".exe" &&
				file.path().extension().string() != ".pdb")
			{
				const auto relPath = std::filesystem::relative(file.path(), FileSystem::GetEnginePath()).parent_path();

				if (!FileSystem::Exists(enginePath / relPath))
				{
					FileSystem::CreateDirectory(enginePath / relPath);
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

	// Copy Scripts folder
	{
		const auto scriptsPath = buildInfo.buildDirectory / "Scripts";
		FileSystem::CreateDirectory(scriptsPath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetScriptsPath()))
		{
			if (!file.is_directory())
			{
				if (file.path().extension() == ".exe" || 
					file.path().extension() == ".pdb" || 
					file.path().extension() == ".rsp")
				{
					continue;
				}

				const auto relPath = std::filesystem::relative(file.path(), FileSystem::GetScriptsPath()).parent_path();

				if (!FileSystem::Exists(scriptsPath / relPath))
				{
					FileSystem::CreateDirectory(scriptsPath / relPath);
				}

				{
					std::scoped_lock lock(myMutex);
					myCurrentFile = file.path().stem().string();
				}

				myCurrentFileNumber++;
				FileSystem::CopyFileTo(file.path(), scriptsPath / relPath);
			}
		}

		// Remove unneeded files
		if (FileSystem::Exists(scriptsPath / "Intermediates"))
		{
			FileSystem::Remove(scriptsPath / "Intermediates");
		}

		if (FileSystem::Exists(scriptsPath / "Volt-ScriptCore.pdb"))
		{
			FileSystem::Remove(scriptsPath / "Volt-ScriptCore.pdb");
		}
	}

	// Copy Project file
	{
		const auto& projectFilePath = Volt::ProjectManager::GetProject().projectFilePath;
		FileSystem::Copy(projectFilePath, buildInfo.buildDirectory / projectFilePath.filename());
	}

	// Copy Assets folder
	{
		const auto assetsDir = buildInfo.buildDirectory / "Assets";
		FileSystem::CreateDirectory(assetsDir);

		for (const auto& file : std::filesystem::recursive_directory_iterator(Volt::ProjectManager::GetAssetsDirectory()))
		{
			if (!file.is_directory())
			{
				const auto assetType = Volt::AssetManager::GetAssetTypeFromPath(file.path());

				if (Utility::ShouldSkipAsset(file.path()) && !Utility::IsEntity(file.path()) && Utility::ShouldSkipFileType(file.path()))
				{
					continue;
				}

				if (assetType == Volt::AssetType::Texture)
				{
					if (!Utility::IsTexturePow2(file.path()))
					{
						VT_CORE_ERROR("[Build] Texture {0} was not power of 2!", file.path().string());
						continue;
					}
				}

				const auto relPath = std::filesystem::relative(file.path(), Volt::ProjectManager::GetAssetsDirectory()).parent_path();

				if (!FileSystem::Exists(assetsDir / relPath))
				{
					FileSystem::CreateDirectory(assetsDir / relPath);
				}

				{
					std::scoped_lock lock(myMutex);
					myCurrentFile = file.path().stem().string();
				}

				myCurrentFileNumber++;
				FileSystem::CopyFileTo(file.path(), assetsDir / relPath);
			}
		}

		// Copy asset registry
		myCurrentFileNumber++;
		FileSystem::CopyFileTo(Volt::ProjectManager::GetAssetsDirectory() / "AssetRegistry.vtreg", assetsDir);
	}

	// Copy Binaries
	{
		const auto binariesDir = buildInfo.buildDirectory / "Binaries";
		FileSystem::CreateDirectory(binariesDir);

		for (const auto& file : std::filesystem::recursive_directory_iterator(Volt::ProjectManager::GetDirectory() / "Binaries"))
		{
			if (!file.is_directory())
			{
				if (file.path().extension() == ".pdb")
				{
					continue;
				}

				{
					std::scoped_lock lock(myMutex);
					myCurrentFile = file.path().stem().string();
				}

				myCurrentFileNumber++;
				FileSystem::CopyFileTo(file.path(), binariesDir);
			}
		}
	}

	// Generate scene dependencies
	{
		for (const auto& handle : buildInfo.sceneHandles)
		{
			const std::filesystem::path path = Volt::AssetManager::GetFilePathFromAssetHandle(handle);

			const auto targetScenePath = buildInfo.buildDirectory / path;

			const Volt::AssetHandle sceneHandle = Volt::AssetManager::GetAssetHandleFromFilePath(path);
			const bool wasLoaded = Volt::AssetManager::IsLoaded(sceneHandle);

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
								if (assetHandle != Volt::Asset::Null() && Volt::AssetManager::ExistsInRegistry(assetHandle))
								{
									sceneDependencies.emplace(assetHandle);
								}
							}
						}
					}
				}
			}

			for (const auto& [instanceId, fieldMap] : scene->GetScriptFieldCache().GetCache())
			{
				for (const auto& [name, instance] : fieldMap)
				{
					if (!instance)
					{
						continue;
					}

					if (!Volt::MonoScriptClass::IsAsset(instance->field.type))
					{
						continue;
					}

					uint64_t assetHandle = *instance->data.As<uint64_t>();
					sceneDependencies.emplace(assetHandle);
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

			if (!wasLoaded)
			{
				Volt::AssetManager::Get().Unload(sceneHandle);
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
		FileSystem::CreateDirectory(enginePath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(FileSystem::GetEnginePath()))
		{
			const auto filePath = file.path().string();

			if (!file.is_directory() && !Utility::StringContains(filePath, "HLSL"))
			{
				result++;
			}
		}
	}

	// Assets folder
	{
		const auto assetsPath = buildInfo.buildDirectory / "Assets";
		FileSystem::CreateDirectory(assetsPath);

		for (const auto& file : std::filesystem::recursive_directory_iterator(Volt::ProjectManager::GetAssetsDirectory()))
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
