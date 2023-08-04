#pragma once
#include "Nexus/API/API.h"
#include "Nexus/Utility/tsdeque.h"
#include "Nexus/Core/Socket.h"
#include "Nexus/Core/Address.h"
#include "Nexus/Packet/Packet.h"
#include "Guarantor.h"
namespace Nexus
{
	class Relay
	{
	public:
		Relay(tsdeque<std::pair<Nexus::Address, Nexus::Packet>>& _packetQueue);
		~Relay();

		void Transmit(Nexus::Packet _packet, Nexus::Address _sockAddr);

		void InitSocket(unsigned short _port);
		void StartBackend();
		void StopBackend();

		unsigned short GetBoundPort() { return m_boundPort; }
		bool IsRunning() { return m_isRunning; }

	private:
		void HandleOutgoing();
		void ReadIncomming();

		Guarantor m_guarantor;

		unsigned short m_boundPort = 0;
		Nexus::Socket m_socket;
		std::thread m_backendThreadIn;
		std::thread m_backendThreadOut;
		std::thread m_backendThreadGuarantor;
		std::atomic<bool> m_isRunning = false;

		tsdeque<std::pair<Nexus::Address, Nexus::Packet>> m_packetQueueOut;
		tsdeque<std::pair<Nexus::Address, Nexus::Packet>>& ex_packetQueueIn;
	};
}
