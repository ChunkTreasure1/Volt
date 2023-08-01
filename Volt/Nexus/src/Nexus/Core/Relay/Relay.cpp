#include "nexuspch.h"
#include "Relay.h"
#include "Nexus/Core/Session.h"

namespace Nexus
{
	Relay::Relay(tsdeque<std::pair<Nexus::Address, Packet>>& in_packetQueue)
		: ex_packetQueueIn(in_packetQueue), m_guarantor(m_packetQueueOut, m_isRunning)
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
			m_socket.SocketError();
			return;
		}

		if (!Nexus::Session::IsValid())
		{
			Nexus::Session::ErrorPrint();
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
		m_backendThreadGuarantor = std::thread([&]() { m_guarantor.Update(); });
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
		m_backendThreadGuarantor.join();

		m_packetQueueOut.clear();
		ex_packetQueueIn.clear();
	}

	void Relay::Transmit(Packet in_packet, Nexus::Address in_sockAddr)
	{
		in_packet.notifyId = m_guarantor.Add(in_sockAddr, in_packet);
		m_packetQueueOut.push_back({ in_sockAddr, in_packet });
	}

	void Relay::HandleOutgoing()
	{
		while (m_isRunning)
		{
			while (!m_packetQueueOut.empty())
			{
				auto _pair = m_packetQueueOut.pop_front();
				m_socket.Send(_pair.first, _pair.second.WGet().data(), (int)_pair.second.Size());
			}
		}
	}

	void Relay::ReadIncomming()
	{
		char byteBuffer[PACKET_SIZE];
		int packetSize;

		while (m_isRunning)
		{
			Nexus::Address addr;
			m_socket.Recieve(addr, byteBuffer, packetSize, PACKET_SIZE);
			if (packetSize > 0)
			{
				auto packet = ConstructPacket(byteBuffer, packetSize);
				if (m_guarantor.Handle(packet, addr))
				{
					ex_packetQueueIn.push_back({ addr,packet });
				}
			}
		}
	}
}
