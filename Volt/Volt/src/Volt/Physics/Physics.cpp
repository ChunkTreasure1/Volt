#include "vtpch.h"
#include "Physics.h"

#include "Volt/Core/Application.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXDebugger.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/MeshColliderCache.h"

#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Utility/FileSystem.h"

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	void Physics::Initialize()
	{
		PhysXInternal::Initialize();
		PhysicsLayerManager::AddLayer("Default");
		MeshColliderCache::Initialize();
	}

	void Physics::Shutdown()
	{
		myScene = nullptr;
		MeshColliderCache::Shutdown();
		PhysXInternal::Shutdown();
	}

	void Physics::LoadSettings()
	{
		if (!FileSystem::Exists(FileSystem::GetPhysicsSettingsPath()))
		{
			return;
		}

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(FileSystem::GetPhysicsSettingsPath()))
		{
			return;
		}

		streamReader.EnterScope("PhysicsSettings");

		mySettings.gravity = streamReader.ReadAtKey("gravity", glm::vec3(0.f, -9.81f, 0.f));
		mySettings.worldBoundsMin = streamReader.ReadAtKey("worldBoundsMin", glm::vec3{ -100.f });
		mySettings.worldBoundsMax = streamReader.ReadAtKey("worldBoundsMax", glm::vec3{ 100.f });
		mySettings.worldBoundsSubDivisions = streamReader.ReadAtKey("worldBoundsSubDivisions", 2);
		mySettings.solverIterations = streamReader.ReadAtKey("solverIterations", 8);
		mySettings.solverVelocityIterations = streamReader.ReadAtKey("solverVelocityIterations", 2);
		mySettings.broadphaseAlgorithm = (BroadphaseType)streamReader.ReadAtKey("broadphaseAlgorithm", (uint32_t)BroadphaseType::AutomaticBoxPrune);
		mySettings.frictionModel = (FrictionType)streamReader.ReadAtKey("frictionModel", (uint32_t)FrictionType::Patch);

		streamReader.ExitScope();
	}

	void Physics::SaveSettings()
	{
		if (!FileSystem::Exists(FileSystem::GetPhysicsSettingsPath().parent_path()))
		{
			std::filesystem::create_directories(FileSystem::GetPhysicsSettingsPath().parent_path());
		}

		YAMLFileStreamWriter streamWriter{ FileSystem::GetPhysicsSettingsPath() };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("PhysicsSettings");

		streamWriter.SetKey("gravity", mySettings.gravity);
		streamWriter.SetKey("broadphaseAlgorithm", (uint32_t)mySettings.broadphaseAlgorithm);
		streamWriter.SetKey("frictionModel", (uint32_t)mySettings.frictionModel);
		streamWriter.SetKey("worldBoundsMin", mySettings.worldBoundsMin);
		streamWriter.SetKey("worldBoundsMax", mySettings.worldBoundsMax);
		streamWriter.SetKey("worldBoundsSubDivisions", mySettings.worldBoundsSubDivisions);
		streamWriter.SetKey("solverIterations", mySettings.solverIterations);
		streamWriter.SetKey("solverVelocityIterations", mySettings.solverVelocityIterations);

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}

	void Physics::LoadLayers()
	{
		if (!FileSystem::Exists(FileSystem::GetPhysicsLayersPath()))
		{
			return;
		}

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(FileSystem::GetPhysicsLayersPath()))
		{
			return;
		}

		PhysicsLayerManager::Clear();

		streamReader.ForEach("PhysicsLayers", [&]() 
		{
			PhysicsLayer layer{};

			layer.layerId = streamReader.ReadAtKey("layerId", 0);
			layer.bitValue = streamReader.ReadAtKey("bitValue", 0);
			layer.collidesWith = streamReader.ReadAtKey("collidesWith", 0);
			layer.name = streamReader.ReadAtKey("name", std::string());

			PhysicsLayerManager::AddLayer(layer);
		});
	}

	void Physics::SaveLayers()
	{
		if (!FileSystem::Exists(FileSystem::GetPhysicsLayersPath().parent_path()))
		{
			std::filesystem::create_directories(FileSystem::GetPhysicsLayersPath().parent_path());
		}

		YAMLFileStreamWriter streamWriter{ FileSystem::GetPhysicsLayersPath() };

		streamWriter.BeginMap();
		streamWriter.BeginSequence("PhysicsLayers");

		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			streamWriter.BeginMap();

			streamWriter.SetKey("layerId", layer.layerId);
			streamWriter.SetKey("bitValue", layer.bitValue);
			streamWriter.SetKey("collidesWith", layer.collidesWith);
			streamWriter.SetKey("name", layer.name);

			streamWriter.EndMap();
		}

		streamWriter.EndSequence();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}

	void Physics::CreateScene(Scene* scene)
	{
		myScene = CreateRef<PhysicsScene>(mySettings, scene);

		if (!Application::Get().IsRuntime())
		{
			if (mySettings.debugOnPlay && !PhysXDebugger::IsDebugging())
			{
				PhysXDebugger::StartDebugging("PhysXDebugInfo", mySettings.debugType == DebugType::LiveDebug);
			}
		}
	}

	void Physics::DestroyScene()
	{
		if (!Application::Get().IsRuntime())
		{
			if (mySettings.debugOnPlay)
			{
				PhysXDebugger::StopDebugging();
			}
		}

		myScene->Destroy();
		myScene = nullptr;
	}

	void Physics::CreateActors(Scene* scene)
	{
		auto& registry = scene->GetRegistry();

		// Rigidbodies
		{
			auto view = registry.view<const RigidbodyComponent>();
			view.each([&](const entt::entity id, const RigidbodyComponent&)
			{
				CreateActor(Entity{ id, scene });
			});
		}

		// Character controller
		{
			auto view = registry.view<const CharacterControllerComponent>();
			view.each([&](const entt::entity id, const CharacterControllerComponent&)
			{
				CreateControllerActor(Entity{ id, scene });
			});
		}
	}

	Ref<PhysicsActor> Physics::CreateActor(Entity entity)
	{
		auto existingActor = myScene->GetActor(entity);
		if (existingActor)
		{
			return existingActor;
		}

		Ref<PhysicsActor> actor = myScene->CreateActor(entity);
		return actor;
	}

	Ref<PhysicsControllerActor> Physics::CreateControllerActor(Entity entity)
	{
		auto existingActor = myScene->GetControllerActor(entity);
		if (existingActor)
		{
			return existingActor;
		}

		Ref<PhysicsControllerActor> actor = myScene->CreateControllerActor(entity);
		return actor;
	}
}
