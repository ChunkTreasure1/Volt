#pragma once
#include <Nexus/Interface/NetManager/NetManager.h>
#include <Nexus/Interface/Connection/ConnectionRegistry.h>

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

		void Reload() override;
	private:
		friend class NetPanel;

		void BackendUpdate() override;

		void OnConnect() override;
		void OnConnectionConfirmed() override;
		void OnDisconnect() override;
		void OnUpdate() override;

		void OnReload() override;

		void OnCreateEntity() override;
		void OnDestroyEntity() override;
		void OnConstructRegistry() override;

		void OnComponentUpdate() override;
		void OnRPC() override;

		void OnEvent() override;

		void OnChatMessage() override;
		void OnMoveUpdate() override;

		void OnPing() override;
		void OnBadPacket() override;

		void CreateDebugEnemy();
		bool m_reload = false;
	};
}
