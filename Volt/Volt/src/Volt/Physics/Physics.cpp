#include "vtpch.h"
#include "Physics.h"

#include "Volt/Core/Application.h"

#include "Volt/Physics/PhysXInternal.h"
#include "Volt/Physics/PhysXDebugger.h"
#include "Volt/Physics/PhysicsScene.h"
#include "Volt/Physics/MeshColliderCache.h"

#include "Volt/Physics/PhysicsLayer.h"

#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Utility/FileSystem.h"

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

		std::ifstream file(FileSystem::GetPhysicsSettingsPath());
		if (!file.is_open())
		{
			return;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root = YAML::Load(sstream.str());
		YAML::Node settingsNode = root["PhysicsSettings"];

		VT_DESERIALIZE_PROPERTY(gravity, mySettings.gravity, settingsNode, gem::vec3(0.f, -981.f, 0.f));
		VT_DESERIALIZE_PROPERTY(worldBoundsMin, mySettings.worldBoundsMin, settingsNode, gem::vec3(-100000.f));
		VT_DESERIALIZE_PROPERTY(worldBoundsMax, mySettings.worldBoundsMax, settingsNode, gem::vec3(100000.f));
		VT_DESERIALIZE_PROPERTY(worldBoundsSubDivisions, mySettings.worldBoundsSubDivisions, settingsNode, 2);
		VT_DESERIALIZE_PROPERTY(solverIterations, mySettings.solverIterations, settingsNode, 8);
		VT_DESERIALIZE_PROPERTY(solverVelocityIterations, mySettings.solverVelocityIterations, settingsNode, 2);

		mySettings.broadphaseAlgorithm = settingsNode["broadphaseAlgorithm"] ? (BroadphaseType)settingsNode["broadphaseAlgorithm"].as<uint32_t>() : BroadphaseType::AutomaticBoxPrune;
		mySettings.frictionModel = settingsNode["frictionModel"] ? (FrictionType)settingsNode["frictionModel"].as<uint32_t>() : FrictionType::Patch;
	}

	void Physics::SaveSettings()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PhysicsSettings" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(gravity, mySettings.gravity, out);
			VT_SERIALIZE_PROPERTY(broadphaseAlgorithm, (uint32_t)mySettings.broadphaseAlgorithm, out);
			VT_SERIALIZE_PROPERTY(frictionModel, (uint32_t)mySettings.frictionModel, out);
			VT_SERIALIZE_PROPERTY(worldBoundsMin, mySettings.worldBoundsMin, out);
			VT_SERIALIZE_PROPERTY(worldBoundsMax, mySettings.worldBoundsMax, out);
			VT_SERIALIZE_PROPERTY(worldBoundsSubDivisions, mySettings.worldBoundsSubDivisions, out);
			VT_SERIALIZE_PROPERTY(solverIterations, mySettings.solverIterations, out);
			VT_SERIALIZE_PROPERTY(solverVelocityIterations, mySettings.solverVelocityIterations, out);
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		if (!FileSystem::Exists(FileSystem::GetPhysicsSettingsPath().parent_path()))
		{
			std::filesystem::create_directories(FileSystem::GetPhysicsSettingsPath().parent_path());
		}

		std::ofstream fout(FileSystem::GetPhysicsSettingsPath());
		fout << out.c_str();
		fout.close();
	}

	void Physics::LoadLayers()
	{
		if (!FileSystem::Exists(FileSystem::GetPhysicsLayersPath()))
		{
			return;
		}

		std::ifstream file(FileSystem::GetPhysicsLayersPath());
		if (!file.is_open())
		{
			return;
		}

		PhysicsLayerManager::Clear();

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root = YAML::Load(sstream.str());
		YAML::Node layersNode = root["PhysicsLayers"];

		for (const auto& node : layersNode)
		{
			PhysicsLayer layer{};

			VT_DESERIALIZE_PROPERTY(layerId, layer.layerId, node, 0);
			VT_DESERIALIZE_PROPERTY(bitValue, layer.bitValue, node, 0);
			VT_DESERIALIZE_PROPERTY(collidesWith, layer.collidesWith, node, 0);
			VT_DESERIALIZE_PROPERTY(name, layer.name, node, std::string());

			PhysicsLayerManager::AddLayer(layer);
		}
	}

	void Physics::SaveLayers()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "PhysicsLayers" << YAML::BeginSeq;
		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(layerId, layer.layerId, out);
			VT_SERIALIZE_PROPERTY(bitValue, layer.bitValue, out);
			VT_SERIALIZE_PROPERTY(collidesWith, layer.collidesWith, out);
			VT_SERIALIZE_PROPERTY(name, layer.name, out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		if (!FileSystem::Exists(FileSystem::GetPhysicsLayersPath().parent_path()))
		{
			std::filesystem::create_directories(FileSystem::GetPhysicsLayersPath().parent_path());
		}

		std::ofstream fout(FileSystem::GetPhysicsLayersPath());
		fout << out.c_str();
		fout.close();
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
		scene->GetRegistry().ForEach<RigidbodyComponent>([&](Wire::EntityId id, const RigidbodyComponent& comp)
		{
			CreateActor(Entity{ id, scene });
		});

		scene->GetRegistry().ForEach<CharacterControllerComponent>([&](Wire::EntityId id, const CharacterControllerComponent& comp)
		{
			CreateControllerActor(Entity{ id, scene });
		});
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
