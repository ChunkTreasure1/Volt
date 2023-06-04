#include "vtpch.h"
#include "NetServer.h"

#include <Nexus/Utility/Types.h>
#include <Nexus/Winsock/AddressHelpers.hpp>

#include <Volt/Scene/SceneManager.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Prefab.h>

#include <Volt/Net/Replicated/RepEntity.h>
#include <Volt/Net/Replicated/RepVariable.h>

#include <Volt/Net/Serialization/NetSerialization.h>

#include <Volt/Net/SceneInteraction/NetActorComponent.h>
#include "Volt/Net/SceneInteraction/GameModeComponent.h"

#include "Volt/Net/Event/NetEventContainer.h"
#include "Volt/Core/Application.h"

namespace Volt
{
	NetServer::NetServer()
	{
		m_id = 0;
	}

	NetServer::~NetServer()
	{
	}

	void NetServer::Update()
	{
		HandleIncomming();

		for (auto& repId : m_registry.GetAllType(Nexus::TYPE::eReplicatedType::VARIABLE))
		{
			auto repVariable = reinterpret_pointer_cast<RepVariable>(m_registry.Get(repId));
			if (repVariable->GetField().netData.replicatedCondition != eRepCondition::CONTINUOUS) continue;
			auto variablePacket = SerializeVariablePacket(*(RepVariable*)m_registry.Get(repId).get(), repId);
			Transmit(variablePacket);
			//AddPacketToIncomming(variablePacket);
			/*for (const auto& connection : m_connectionRegistry.GetClientIDs())
			{
				m_relay.Transmit(variablePacket, connection.second);
			}*/
		}

		for (auto& repId : m_registry.GetAllType(Nexus::TYPE::eReplicatedType::ENTITY))
		{
			auto ptr = m_registry.Get(repId);
			auto repEntity = *reinterpret_pointer_cast<RepEntity>(ptr);
			auto entity = Entity(repEntity.GetEntityId(), SceneManager::GetActiveScene().lock().get());
			if (entity.IsNull()) continue;

			const auto& pawnComp = entity.GetComponent<NetActorComponent>();
			if (pawnComp.condition != eRepCondition::CONTINUOUS) continue;

			if (pawnComp.updateTransform)
			{
				auto transformPacket = SerializeTransformPacket(entity.GetId(), repId);
				for (const auto& connection : m_connectionRegistry.GetClientIDs())
				{
					if (connection.first == repEntity.GetOwner()) continue;
					m_relay.Transmit(transformPacket, connection.second);
				}
			}
		}
	}

	void NetServer::Transmit(const Nexus::Packet& in_packet)
	{
		auto host = Application::Get().GetNetHandler().GetBackend()->GetClientId();
		for (const auto& connection : m_connectionRegistry.GetClientIDs())
		{
			if (connection.first == host) continue;
			m_relay.Transmit(in_packet, connection.second);
		}
	}

	void NetServer::Shutdown()
	{
		m_connectionRegistry.Clear();
		GetRegistry().Clear();
		NetManager::Shutdown();
	}
	void NetServer::Init()
	{
		Volt::Application::Get().GetNetHandler().LoadNetScene();
	}
#pragma optimize("", off)
	void NetServer::OnConnect()
	{
		//CreateDebugEnemy();

		auto packet = m_currentPacket.second;
		auto sockAddr = m_currentPacket.first;

		Nexus::TYPE::CLIENT_ID newClientID = m_connectionRegistry.AddConnection(sockAddr);
		std::string str = packet.GetString(static_cast<int32_t>(packet.body.size()));

		// Connect client
		m_connectionRegistry.UpdateAlias(newClientID, str);

		Nexus::TYPE::REP_ID registerEntityId = Nexus::RandRepID();
		while (m_registry.IdExist(registerEntityId)) registerEntityId = Nexus::RandRepID();

		auto gameMode = SceneManager::GetActiveScene().lock()->GetAllEntitiesWith<GameModeComponent>();
		if (gameMode.size() != 1)
		{
			VT_CORE_CRITICAL("Something went wrong with GameModeComponent. Make sure there is only one in the scene");
			return;
		}
		auto gameModeEnt = Entity(gameMode[0], SceneManager::GetActiveScene().lock().get());
		auto gameModeComp = gameModeEnt.GetComponent<GameModeComponent>();
		//auto spawnPointEnt = Entity(gameModeEnt.spawnPoint, SceneManager::GetActiveScene().get()).GetComponent<GameModeComponent>();

		auto handle = gameModeComp.prefabHandle;
		auto prefabData = CreatePrefabData(registerEntityId, newClientID, handle);
		auto spawnPointData = CreateTransformComponentData(0, gameModeEnt.GetComponent<TransformComponent>());
		prefabData.componentData.push_back(CreateRef<RepTransformComponentData>(spawnPointData));
		ConstructPrefab(prefabData, m_registry);
		//Entity(m_registry.GetAs<RepEntity>(registerEntityId)->GetEntityId(), SceneManager::GetActiveScene().get()).SetPosition({ 0,0,0 });

		// Confirmation
		Nexus::Packet confirmationPacket;
		confirmationPacket.ownerID = newClientID;
		confirmationPacket.id = Nexus::ePacketID::CONNECTION_CONFIRMED;
		confirmationPacket < spawnPointData < prefabData;
		m_relay.Transmit(confirmationPacket, m_connectionRegistry.GetSockAddr(newClientID));

		// Sync registry on new connection
		for (auto repId : m_registry.GetAllType(Nexus::TYPE::eReplicatedType::ENTITY))
		{
			auto repEnt = m_registry.GetAs<RepEntity>(repId);

			if (repEnt->GetOwner() == newClientID) continue;
			if (repEnt->GetHandle() == 0) continue;
			if (repEnt->GetPreplaced()) continue;

			auto sceneEnt = Entity(repEnt->GetEntityId(), SceneManager::GetActiveScene().lock().get());
			Nexus::Packet syncPacket;
			syncPacket.id = Nexus::ePacketID::CREATE_ENTITY;
			syncPacket < CreateTransformComponentData(repId, sceneEnt.GetComponent<TransformComponent>());
			syncPacket < CreatePrefabData(repId, repEnt->GetOwner(), repEnt->GetHandle());

			m_relay.Transmit(syncPacket, m_connectionRegistry.GetSockAddr(newClientID));
		}

		Nexus::Packet entCreationPacket;
		entCreationPacket.ownerID = 0;
		entCreationPacket.id = Nexus::ePacketID::CREATE_ENTITY;
		entCreationPacket < prefabData;

		auto hostId = Application::Get().GetNetHandler().GetBackend()->GetClientId();
		for (const auto& _pair : m_connectionRegistry.GetClientIDs())
		{
			if (_pair.first == newClientID || _pair.first == hostId) continue;
			m_relay.Transmit(entCreationPacket, _pair.second);
		}

		LogTrace("User: " + m_connectionRegistry.GetAlias(newClientID) + " has joined");
	}
#pragma optimize("", on)

	void NetServer::OnConnectionConfirmed()
	{
		// Should never be called
		OnBadPacket();
	}

	void NetServer::OnDisconnect()
	{
		auto packet = m_currentPacket.second;

		for (auto repId : m_registry.GetAllOwner(packet.ownerID))
		{
			auto ent = reinterpret_cast<RepEntity*>(m_registry.Get(repId).get())->GetEntityId();
			Volt::SceneManager::GetActiveScene().lock()->RemoveEntity(Entity(ent, SceneManager::GetActiveScene().lock().get()));
			m_registry.Unregister(repId);
		}

		LogTrace("User: " + m_connectionRegistry.GetAlias(packet.ownerID) + " has disconnected");
		m_connectionRegistry.RemoveConnection(packet.ownerID);
	}

	void NetServer::OnUpdate()
	{
		auto packet = m_currentPacket.second;
		while (auto repData = DeserializePacketBody(packet))
		{
			ApplyComponentData(repData, m_registry);
		}
		Transmit(*&m_currentPacket.second);
	}

	void NetServer::OnCreateEntity()
	{
		auto packet = m_currentPacket.second;
		auto repData = DeserializePacketBody(packet);
		auto prefabData = *reinterpret_cast<RepPrefabData*>(repData.get());
		prefabData.repId = m_registry.GetNewId();
		ConstructPrefab(prefabData, m_registry);

		Nexus::Packet newPacket;
		newPacket.ownerID = packet.ownerID;
		newPacket.id = packet.id;
		for (auto comp : prefabData.componentData)
		{
			if (comp->dataType == eNetSerializerDescriptor::TRANSFORM) newPacket < *(RepTransformComponentData*)&comp;
		}
		newPacket < prefabData;
		Transmit(newPacket);
	}

	void NetServer::OnDestroyEntity()
	{
		auto packet = m_currentPacket.second;
		Nexus::TYPE::REP_ID repId;
		packet >> repId;

		auto repEnt = m_registry.GetAs<RepEntity>(repId);
		if (!repEnt)
		{
			VT_CORE_ERROR("Failed to destroy net ent, Server");
			return;
		}
		// find connected variables
		// validation

		m_registry.Unregister(repId);
		auto scene = SceneManager::GetActiveScene();
		auto scenePtr = scene.lock();

		scenePtr->RemoveEntity(Entity(repEnt->GetEntityId(), scenePtr.get()));

		auto tPack = m_currentPacket.second;
		tPack.ownerID = 0;
		Transmit(tPack);
		// errors
	}

	void NetServer::OnMoveUpdate()
	{
		auto packet = m_currentPacket.second;
		if (auto repData = DeserializePacketBody(packet))
		{
			if (ApplyComponentData(repData, m_registry)) return;
		}
	}

	void NetServer::OnComponentUpdate()
	{

	}

	void NetServer::OnRPC()
	{

	}

	void NetServer::OnEvent()
	{
		NetEvent event;
		m_currentPacket.second > event;
		NetEventContainer::StaticAddOutgoing(event);
	}

	void NetServer::OnChatMessage()
	{
		auto packet = m_currentPacket.second;

		// #Nexus_TEMP: Should be moved into packet, just read first bytes to get len of new string
		std::string str;
		str.resize(packet.body.size());
		memcpy_s(str.data(), str.size(), packet.body.data(), packet.body.size());
		std::cout << str << "\n";

		for (const auto& client : m_connectionRegistry.GetClientIDs())
		{
			if (m_connectionRegistry.GetSockAddr(packet.ownerID) != client.second)
			{
				m_relay.Transmit(packet, client.second);
			}
		}
	}

	void NetServer::OnPing()
	{
		auto packet = m_currentPacket.second;
		m_relay.Transmit(packet, m_connectionRegistry.GetSockAddr(packet.ownerID));
	}

	void NetServer::OnBadPacket()
	{
		std::string ip = Nexus::GetIp(m_currentPacket.first);
		// Check if IP exists in alias map
		LogError("Bad Packet Recieved on Server from IP: " + ip);
	}

	void NetServer::CreateDebugEnemy()
	{
		auto gameMode = SceneManager::GetActiveScene().lock()->GetAllEntitiesWith<GameModeComponent>();
		if (gameMode.size() != 1)
		{
			VT_CORE_CRITICAL("Something went wrong with GameModeComponent. Make sure there is only one in the scene");
			return;
		}
		auto handle = Entity(gameMode[0], SceneManager::GetActiveScene().lock().get()).GetComponent<GameModeComponent>().enemy;
		auto prefabData = CreatePrefabData(10, 0, handle);

		ConstructPrefab(prefabData, m_registry);
	}
}
