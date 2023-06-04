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

		m_isRunning = true;
		m_backendThread = std::thread([&]() { ReadIncomming(); });
	}

	void Relay::StopBackend()
	{
		if (!m_isRunning)
		{
			return;
		}
		m_isRunning = false;
		char data[256];
		std::fill_n(data, 256, (char)ePacketID::CLOSE);
		m_socket.SendTo("127.0.0.1", m_boundPort, data, 10);
		m_backendThread.join();
		m_socket.Close();
		ex_packetQueueIn.clear();
	}

	void Relay::Transmit(Packet in_packet, sockaddr_in in_sockAddr)
	{
		m_socket.SendTo(in_sockAddr, in_packet.WGet().data(), (int)in_packet.Size());
	}

	void Relay::HandleOutgoing()
	{

	}

	void Relay::ReadIncomming()
	{
		char byteBuffer[PACKET_SIZE];
		int packetSize;

		while (m_isRunning)
		{
			auto addr = m_socket.RecvFrom(byteBuffer, packetSize, PACKET_SIZE);
			auto packet = ConstructPacket(byteBuffer, packetSize);
			ex_packetQueueIn.push_back({ addr,packet });
		}
	}
}
