#include "vtpch.h"
#include "NetHandler.h"
#include "Server/NetServer.h"
#include "Client/NetClient.h"
#include "Volt/Net/SceneInteraction/NetContract.h"
#include "Volt/Net/Serialization/NetSerialization.h"
#include <Nexus/Winsock/AddressHelpers.hpp>
#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include <Nexus/Interface/Replication/ReplicationRegistry.h>

#include "Volt/Scene/SceneManager.h"
#include "Volt/Net/SceneInteraction/NetActorComponent.h"
#include "Volt/Net/Replicated/RepEntity.h"

#include "Volt/Core/Profiling.h"
namespace Volt
{
	NetHandler::NetHandler()
	{
		m_lastUnhandledNetError = new eNetErrorCode();
		NetContractContainer::Load();
	}

	void NetHandler::StartSinglePlayer()
	{
		StartServer(27018);
		//netHandler.m_backend->Start();
		//netHandler.m_backend->StoreServerData(m_netSettings.connectAddress, m_netSettings.hostingPort);

		Nexus::Packet connectionPacket;
		connectionPacket.id = Nexus::ePacketID::CONNECT;
		connectionPacket << std::string("Host Client");
		m_backend->GetIncommingPacketQueue().push_back({ Nexus::CreateSockAddr("127.0.0.1", m_backend->GetRelay().GetBoundPort()),connectionPacket });
	}

	NetHandler::~NetHandler()
	{
		delete m_lastUnhandledNetError;
	}

	void NetHandler::Update(const float& deltaTime)
	{
		if (!m_backend) return;
		//if (MonoScriptEngine::IsRunning()) LoadNetScene();
		if (!m_handleTick) return;
		m_backend->HandleTick(deltaTime);

		NetEvent e = NetEvent(eNetEvent::NIL, 0, std::vector<uint8_t>());
		while (m_eventContainer.GetIncomming(e))
		{
			NetContractContainer::Execute(e.GetId(), e.GetEvent(), e.GetData());
			if (IsHost())
			{
				auto ep = SerializeNetEventPacket(e);
				m_backend->Transmit(ep);
			}
		}
		while (m_eventContainer.GetOutgoing(e))
		{
			auto ep = SerializeNetEventPacket(e);
			if (IsHost()) m_backend->AddPacketToIncomming(ep);
			else m_backend->Transmit(ep);
		}
	}

	void NetHandler::Stop()
	{
		if (!m_backend) return;
		m_backend->Shutdown();
		Nexus::WSA::Session::Clean();
		m_netSceneLoaded = false;
	}

	void NetHandler::Disconnect()
	{

	}

	bool NetHandler::IsRunning()
	{
		if (!m_backend)return false;
		return m_backend->GetRelay().IsRunning();
	}

	eNetErrorCode* NetHandler::GetErrorPtr()
	{
		return m_lastUnhandledNetError;
	}

	NetEventContainer& NetHandler::GetEventContainer()
	{
		return m_eventContainer;
	}

	Scope<Nexus::NetManager>& NetHandler::GetBackend()
	{
		return m_backend;
	}

	bool NetHandler::IsOwner(Nexus::TYPE::REP_ID in_object)
	{
		auto repObjectOwner = m_backend->GetRegistry().Get(in_object)->GetOwner();
		if (IsHost() && repObjectOwner == 0) return true;
		return repObjectOwner == m_backend->GetClientId();
	}

	bool NetHandler::IsHost()
	{
		return m_isHost;
	}

	void NetHandler::LoadNetScene()
	{
		if (m_netSceneLoaded || !m_backend) return;
		/*for (auto id : m_backend->GetRegistry().GetAllType(Nexus::TYPE::eReplicatedType::ENTITY))
		{
			auto repEnt = m_backend->GetRegistry().GetAs<Volt::RepEntity>(id);
			auto beginId = id;

			RecursiveHandleMono(repEnt->GetEntityId(), id, beginId, &m_backend->m_registry);
		}*/
		// #nexus_todo: Check ids here
		m_netSceneLoaded = true;
	}

	void NetHandler::OnEvent(Volt::Event& in_event)
	{
		if (!m_backend)return;
		if (in_event.GetEventType() == Volt::EventType::OnSceneLoaded)
		{
			m_backend->m_registry.Clear();
			for (auto entId : SceneManager::GetActiveScene().lock().get()->GetAllEntitiesWith<NetActorComponent>())
			{
				auto sceneEnt = Entity(entId, SceneManager::GetActiveScene().lock().get());
				auto netComp = sceneEnt.GetComponent<NetActorComponent>();
				auto prefabComp = sceneEnt.GetComponent<PrefabComponent>();
				auto repEnt = Volt::RepEntity(entId, netComp.clientId, prefabComp.prefabAsset, true);
				// #nexus_todo: fix components to be hosted by host
				m_backend->GetRegistry().Register((Nexus::TYPE::REP_ID)netComp.repId, repEnt);

				Volt::RepPrefabData data;
				data.repId = netComp.repId;
				data.handle = repEnt.GetHandle();
				RecursiveOwnerShipControll(repEnt.GetEntityId(), data);
			}
		}
		if (in_event.GetEventType() == Volt::EventType::OnScenePlay)
		{
			for (auto id : m_backend->GetRegistry().GetAllType(Nexus::TYPE::eReplicatedType::ENTITY))
			{
				auto repEnt = m_backend->GetRegistry().GetAs<Volt::RepEntity>(id);
				auto beginId = id;

				RecursiveHandleMono(repEnt->GetEntityId(), id, beginId, &m_backend->m_registry, false);
			}
			m_handleTick = true;
		}
	}

	void NetHandler::StartClient(uint16_t port)
	{
		if (m_backend)
			m_backend->Shutdown();
		m_isHost = false;
		m_backend = CreateScope<NetClient>();
		Nexus::WSA::Session::Start();
		m_backend->Start(port);
	}

	void NetHandler::StartServer(uint16_t port)
	{
		if (m_backend)
			m_backend->Shutdown();
		m_isHost = true;
		m_backend = CreateScope<NetServer>();
		Nexus::WSA::Session::Start();
		m_backend->Start(port);
	}
}
