#pragma once
#include "Nexus/Core/Relay/Relay.h"
#include "Nexus/Core/Packet/Packet.hpp"
#include "Nexus/Interface/Replication/ReplicationRegistry.h"
#include "Nexus/Winsock/ConnectionManager.h"

namespace Volt
{
	class NetHandler;
}

namespace Nexus
{
	class NetManager
	{
	public:
		NetManager();
		~NetManager();

		void Start(uint16_t in_port = 0);
		void Shutdown();

		virtual void Init() = 0;

		void HandleTick(float deltaTime);
		virtual void Transmit(const Nexus::Packet& in_packet) = 0;

		Relay& GetRelay() { return m_relay; }
		ReplicationRegisty& GetRegistry() { return m_registry; }

		tsdeque<std::pair<sockaddr_in, Packet>>& GetIncommingPacketQueue() { return m_packetQueueIn; }

		Nexus::ConnectionManager& GetConnectionRegistry() { return m_connectionRegistry; }

		const Nexus::TYPE::CLIENT_ID& GetClientId() { return m_id; }
		
		//Debug
		void AddPacketToIncomming(const Packet& in_packet);

	protected:
		friend class Volt::NetHandler;
		void HandleIncomming();

		virtual void Update() = 0;

		Relay m_relay;
		tsdeque<std::pair<sockaddr_in, Packet>> m_packetQueueIn;

		ReplicationRegisty m_registry;
		Nexus::ConnectionManager m_connectionRegistry;
		// id 1 is connection id
		Nexus::TYPE::CLIENT_ID m_id = 1;

		float m_timer = 0;

		// Packet management begin
		std::pair<sockaddr_in, Nexus::Packet> m_currentPacket;
		// Connect
		virtual void OnConnect() = 0;
		virtual void OnConnectionConfirmed() = 0;
		virtual void OnDisconnect() = 0;
		virtual void OnUpdate() = 0;

		virtual void OnCreateEntity() = 0;
		virtual void OnDestroyEntity() = 0;

		// RepEntity
		virtual void OnMoveUpdate() = 0;
		virtual void OnComponentUpdate() = 0;
		virtual void OnRPC() = 0;
		virtual void OnEvent() = 0;

		// Chat
		virtual void OnChatMessage() = 0;

		// Misc
		virtual void OnPing() = 0;
		virtual void OnBadPacket() = 0;
		// Packet management end
	};
}
