#include "nexuspch.h"
#include "Manager.h"
#include "Nexus/Core/Session.h"
#include "Nexus/Core/Address.h"


namespace Nexus
{
	Manager::Manager()
		: m_relay(m_packetQueueIn)
	{

	}

	Manager::~Manager()
	{
		m_relay.StopBackend();
	}

	void Manager::Start(uint16_t _port)
	{
		Nexus::Session::Start();
		if (Nexus::Session::IsValid())
		{
			// #KITE_INTERFACE_TODO: Log/Assert
		}

		m_relay.InitSocket(_port);
		m_relay.StartBackend();
	}

	void Manager::Shutdown()
	{
		// #KITE_INTERFACE_TODO: Disconnect all clients
		m_relay.StopBackend();
		GetRegistry().Clear();
		Nexus::Session::Start();
	}

	void Manager::AddPacketToIncomming(const Packet& _packet)
	{
		GetIncommingPacketQueue().push_back({ Nexus::Address(Nexus::Address::ConstructDescription("127.0.0.1", GetRelay().GetBoundPort())), _packet });
	}

	void Manager::HandleTick(float deltaTime)
	{
		m_timer += deltaTime;
		if (m_timer > TICK_LEN)
		{
			BackendUpdate();
			m_timer -= TICK_LEN;
		}
	}

	void Manager::HandleIncomming()
	{
		// #nexus_todo: validate WSA

		while (!m_packetQueueIn.empty())
		{
			m_currentPacket = m_packetQueueIn.pop_front();

			switch (m_currentPacket.second.id)
			{
				case Nexus::ePacketID::CONNECT:OnConnect(); break;
				case Nexus::ePacketID::CONNECTION_CONFIRMED:OnConnectionConfirmed(); break;
				case Nexus::ePacketID::DISCONNECT:OnDisconnect(); break;
				case Nexus::ePacketID::UPDATE:OnUpdate(); break;

				case Nexus::ePacketID::RELOAD:OnReload(); break;
				case Nexus::ePacketID::RELOAD_DENIED:OnReloadDenied(); break;
				case Nexus::ePacketID::RELOAD_CONFIRMED:OnReloadConfirmed(); break;

				case Nexus::ePacketID::CREATE_ENTITY: OnCreateEntity(); break;
				case Nexus::ePacketID::REMOVE_ENTITY: OnDestroyEntity(); break;
				case Nexus::ePacketID::CONSTRUCT_REGISTRY: OnConstructRegistry(); break;

				case Nexus::ePacketID::MOVE: OnMoveUpdate(); break;
				case Nexus::ePacketID::RPC: OnRPC(); break;
				case Nexus::ePacketID::COMPONENT_UPDATE: OnComponentUpdate(); break;
				case Nexus::ePacketID::EVENT: OnEvent(); break;

				case Nexus::ePacketID::CHAT_MESSAGE:OnChatMessage(); break;

				case Nexus::ePacketID::PING: OnPing(); break;
				case Nexus::ePacketID::CLOSE: break;
				default: OnBadPacket(); break;
			}
		}
	}
}
