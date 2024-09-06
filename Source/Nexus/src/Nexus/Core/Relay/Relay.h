#pragma once
#include "Nexus/Utility/tsdeque.h"
#include "Nexus/Winsock/UDPSocket.h"
#include "Nexus/Core/Packet/Packet.hpp"

namespace Nexus
{
	class Relay
	{
	public:
		Relay(tsdeque<std::pair<sockaddr_in, Packet>>& in_packetQueue);
		~Relay();

		void Transmit(Packet in_packet, sockaddr_in in_sockAddr);

		void InitSocket(unsigned short in_port);
		void StartBackend();
		void StopBackend();

		unsigned short GetBoundPort() { return m_boundPort; }
		bool IsRunning() { return m_isRunning; }

	private:
		void HandleOutgoing();
		void ReadIncomming();

		unsigned short m_boundPort = 0;
		UDPSocket m_socket;
		std::thread m_backendThreadIn;
		std::thread m_backendThreadOut;
		std::atomic<bool> m_isRunning = false;
		
		// #nexus_todo: Should probably be a map with data identifier instead.
		// instead of creating new packet in queue for data thats already in there
		// overwrite packet if double
		tsdeque<std::pair<sockaddr_in, Packet>> m_packetQueueOut;
		tsdeque<std::pair<sockaddr_in, Packet>>& ex_packetQueueIn;
		// #IDEA: kite might works as a networking interface only for easy swap to asio at a later date
	};
}
