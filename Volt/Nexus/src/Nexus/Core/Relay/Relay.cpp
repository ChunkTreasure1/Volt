#include "nexuspch.h"
#include "Relay.h"
#include "Nexus/Core/Core.h"

namespace Nexus
{
	Relay::Relay(tsdeque<std::pair<sockaddr_in, Packet>>& in_packetQueue)
		: ex_packetQueueIn(in_packetQueue)
	{

	}

	Relay::~Relay()
	{
		StopBackend();
	}

	void Relay::InitSocket(unsigned short in_port)
	{
		if (m_isRunning)
		{
			LogWarning("Relay already active, Only one session supported");
			return;
		}

		m_socket.Init();
		m_socket.Bind(in_port);
		m_boundPort = in_port;
	}

	void Relay::StartBackend()
	{
		if (!m_socket.IsValid())
		{
			// #nexus_todo: prob not correct error
			m_socket.SocketError(true);
			return;
		}

		if (!WSA::Session::IsValid())
		{
			WSA::Session::ErrorPrint();
			return;
		}

		if (m_isRunning)
		{
			LogWarning("Relay backend already active, Only one thread supported");
			return;
		}

		if (m_isRunning)
		{
			StopBackend();
		}
		m_isRunning = true;
		m_backendThreadIn = std::thread([&]() { ReadIncomming(); });
		m_backendThreadOut = std::thread([&]() { HandleOutgoing(); });
	}

	void Relay::StopBackend()
	{
		if (!m_isRunning)
		{
			return;
		}
		m_isRunning = false;
		m_socket.Close();
		m_backendThreadIn.join();
		m_backendThreadOut.join();
	}

	void Relay::Transmit(Packet in_packet, sockaddr_in in_sockAddr)
	{
		if (!m_socket.SendTo(in_sockAddr, in_packet.WGet().data(), (int)in_packet.Size()))
		{
			LogError("wes");
			return;
		}
		int i = 0;
		i++;
		int c = i;
		//m_packetQueueOut.push_back({ in_sockAddr, in_packet });
	}

	void Relay::HandleOutgoing()
	{
		while (m_isRunning)
		{
			while (!m_packetQueueOut.empty())
			{
				auto _pair = m_packetQueueOut.pop_front();
				/*	if (!m_socket.SendTo(_pair.first, _pair.second.WGet().data(), (int)_pair.second.Size()))
					{

					}*/
			}
		}
	}

	void Relay::ReadIncomming()
	{
		char byteBuffer[PACKET_SIZE];
		int packetSize;

		while (m_isRunning)
		{
			auto addr = m_socket.RecvFrom(byteBuffer, packetSize, PACKET_SIZE);
			if (packetSize > 0)
			{
				auto packet = ConstructPacket(byteBuffer, packetSize);
				ex_packetQueueIn.push_back({ addr,packet });
			}
		}
	}
}
