#pragma once
#include <Nexus/Interface/NetManager/NetManager.h>
#include <Nexus/Winsock/ConnectionManager.h>

namespace Volt
{
	class NetServer : public Nexus::NetManager
	{
	public:
		NetServer();
		~NetServer();

		void Transmit(const Nexus::Packet& in_packet) override;
		void Shutdown();
		void Init() override;
	private:
		friend class NetPanel;

		void Update() override;

		void OnConnect() override;
		void OnConnectionConfirmed() override;
		void OnDisconnect() override;
		void OnUpdate() override;

		void OnCreateEntity() override;
		void OnDestroyEntity() override;

		void OnComponentUpdate() override;
		void OnRPC() override;

		void OnEvent() override;


		void OnChatMessage() override;
		void OnMoveUpdate() override;

		void OnPing() override;
		void OnBadPacket() override;

		void CreateDebugEnemy();
	};
}
