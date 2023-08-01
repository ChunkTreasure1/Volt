#include "vtpch.h"
#include "NetClient.h"
#include <Nexus/Core/Address.h>

//#include "Volt/Net/SceneInteraction/NetPrefabInstantiation.h"
#include "Volt/Net/Serialization/NetSerialization.h"
#include "Volt/Net/Replicated/RepEntity.h"

#include "Volt/Scene/SceneManager.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Scripting/Mono/MonoScriptClass.h"
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Scripting/Mono/MonoScriptInstance.h"
#include "Volt/Net/SceneInteraction/NetActorComponent.h"

#include "Volt/Net/Event/NetEvent.h"
#include "Volt/Core/Application.h"

namespace Volt
{
	NetClient::NetClient()
	{
	}

	NetClient::~NetClient()
	{
	}

	void NetClient::MissingEntity(Nexus::TYPE::REP_ID repId)
	{

	}

	void NetClient::BackendUpdate()
	{
		HandleIncomming();
		for (auto& repId : m_registry.GetAllType(Nexus::TYPE::eReplicatedType::VARIABLE))
		{
			auto repVariable = reinterpret_pointer_cast<RepVariable>(m_registry.Get(repId));
			if (repVariable->GetField().netData.replicatedCondition != eRepCondition::CONTINUOUS) continue;
			auto variablePacket = SerializeVariablePacket(*(RepVariable*)m_registry.Get(repId).get(), repId);
			Transmit(variablePacket);
		}
		for (auto& repId : m_registry.GetAllType(Nexus::TYPE::eReplicatedType::ENTITY))
		{
			auto ptr = m_registry.Get(repId);
			auto repEntity = *reinterpret_pointer_cast<RepEntity>(ptr);
			auto entity = Entity(repEntity.GetEntityId(), SceneManager::GetActiveScene().lock().get());
			if (entity.IsNull()) continue;
			if (repEntity.GetOwner() != m_id) continue;

			auto pawnComp = entity.GetComponent<NetActorComponent>();
			if (pawnComp.condition != eRepCondition::CONTINUOUS) continue;

			if (pawnComp.updateTransform)
			{
				auto transformPacket = SerializeTransformPacket(entity.GetId(), repId);
				Transmit(transformPacket);
				//m_relay.Transmit(transformPacket, Nexus::CreateSockAddr(m_serverAdress, m_serverPort));
			}
		}

		if (m_requestReload)
		{
			Nexus::Packet reloadRequest;
			reloadRequest << m_sceneInstanceId;
			Transmit(reloadRequest);
			m_requestReload = false;
		}
	}

	void NetClient::Transmit(const Nexus::Packet& in_packet)
	{
		auto p = in_packet;
		if (m_id == 1 && p.id != Nexus::ePacketID::CONNECT)
		{
			onLoadedQueue.push(p);
			return;
		}
		p.ownerID = m_id;
		m_relay.Transmit(p, Nexus::Address::ConstructAddress(m_serverAdress, m_serverPort));
	}

	void NetClient::Init()
	{
		Volt::Application::Get().GetNetHandler().LoadNetScene();
	}

	void NetClient::OnConnect()
	{

	}

	void NetClient::OnConnectionConfirmed()
	{
		m_id = m_currentPacket.second.ownerID;
		LogTrace("Connection to server confirmed, CLIENT_ID is: " + std::to_string(m_id));
		while (!onLoadedQueue.empty())
		{
			auto p = onLoadedQueue.front();
			p.ownerID = m_id;
			m_relay.Transmit(p, Nexus::Address::ConstructAddress(m_serverAdress, m_serverPort));
			onLoadedQueue.pop();
		}
	}

	void NetClient::OnDisconnect()
	{

	}

	void NetClient::OnUpdate()
	{
		auto packet = m_currentPacket.second;
		while (auto repData = DeserializePacketBody(packet))
		{
			ApplyComponentData(repData, m_registry);
		}
	}

	void NetClient::OnReloadDenied()
	{
		// request again in x time
		m_requestReload = true;

	}

	void NetClient::OnReloadConfirmed()
	{
		// reload scene
		m_requestReload = false;
		AssetHandle handle = 0;
		m_currentPacket.second >> handle;
		Volt::OnSceneTransitionEvent loadEvent{ handle };
		Volt::Application::Get().OnEvent(loadEvent);
	}

	void NetClient::OnCreateEntity()
	{
		auto& packet = m_currentPacket.second;
		auto data = DeserializePacketBody(packet);
		auto prefabData = reinterpret_pointer_cast<Volt::RepPrefabData>(data);
		if (ConstructPrefab(*prefabData, m_registry)) return;

		// error management;
	}

	void NetClient::OnDestroyEntity()
	{
		auto packet = m_currentPacket.second;
		Nexus::TYPE::REP_ID repId;
		packet >> repId;

		auto repEnt = m_registry.GetAs<RepEntity>(repId);
		if (!repEnt)
		{
			// error
			VT_CORE_ERROR("Failed to destroy net ent, Client");
			return;
		}
		// find connected variables
		// validation

		m_registry.Unregister(repId);
		auto scene = SceneManager::GetActiveScene();
		auto scenePtr = scene.lock();

		scenePtr->RemoveEntity(Entity(repEnt->GetEntityId(), scenePtr.get()));

		// find Entity
		// find connected variables
		// clean reg
		// clean scene
		// errors
	}

	void NetClient::OnConstructRegistry()
	{
		// set netscene instance id
		m_currentPacket.second >> m_sceneInstanceId;
	}

	void NetClient::OnMoveUpdate()
	{
		auto packet = m_currentPacket.second;
		auto repData = DeserializePacketBody(packet);
		if (ApplyComponentData(repData, m_registry)) return;

		// error management
	}

	void NetClient::OnComponentUpdate()
	{
	}

	void NetClient::OnRPC()
	{
		if (!MonoScriptEngine::IsRunning())
		{
			VT_CORE_ERROR("MonoScriptEngine not running in client RPC call");
			return;
		}

		auto packet = m_currentPacket.second;
		auto rpcData = reinterpret_pointer_cast<RepRPCData>(DeserializePacketBody(packet));
		auto netEnt = reinterpret_pointer_cast<RepEntity>(m_registry.Get(rpcData->repId));
		if (!netEnt)
		{
			VT_CORE_ERROR("net entity is null in client RPC call");
			return;
		}

		auto sceneEnt = Entity(netEnt->GetEntityId(), SceneManager::GetActiveScene().lock().get());
		if (sceneEnt.IsNull())
		{
			VT_CORE_ERROR("scene entity is null in client RPC call");
			return;
		}

		auto monoClass = MonoScriptEngine::GetScriptClass(rpcData->monoProject + "." + rpcData->monoClass);
		auto monoMethod = monoClass->GetMethod(rpcData->monoMethod, 0);
		if (!monoMethod)
		{
			VT_CORE_ERROR("Method error in client RPC call");
			return;
		}
		if (!sceneEnt.HasComponent<MonoScriptComponent>())
		{
			VT_CORE_ERROR("missing monoscriptComponent in client RPC call");
			return;
		}
		auto scriptsVector = Entity(sceneEnt.GetId(), SceneManager::GetActiveScene().lock().get()).GetComponent<MonoScriptComponent>().scriptIds;
		Ref<MonoScriptInstance> scrInstance = nullptr;
		for (auto scrID : scriptsVector)
		{
			scrInstance = MonoScriptEngine::GetInstanceFromId(scrID);
			if (!scrInstance) continue;
			if (monoClass == scrInstance->GetClass())
			{
				auto t = MonoScriptEngine::GetInstanceFromId(scrID)->GetHandle();
				MonoScriptEngine::CallMethod(t, monoMethod);
				return;
			}
		}
		VT_CORE_ERROR("script not found in client RPC call");
	}

	void NetClient::OnEvent()
	{
		NetEvent event;
		m_currentPacket.second > event;
		NetEventContainer::StaticAddIncomming(event);
	}

	void NetClient::OnChatMessage()
	{
	}

	void NetClient::OnPing()
	{
	}

	void NetClient::OnBadPacket()
	{
		std::string ip = m_currentPacket.first.IPV4();
		// Check if IP exists in alias map
		LogError("Bad Packet Recieved on Client from IP: " + ip);
	}
}
