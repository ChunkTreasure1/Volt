#pragma once
#include <Nexus/Manager/Manager.h>
#include <queue>
namespace Volt
{
	class NetClient : public Nexus::Manager
	{
	public:
		NetClient();
		~NetClient();

		void StoreServerData(std::string adress, unsigned short port) { m_serverAdress = adress; m_serverPort = port; }

		void Transmit(const Nexus::Packet& in_packet) override;
		void Init() override;

		void Reload() override { m_requestReload = true; }

	private:
		friend class NetPanel;

		void MissingEntity(Nexus::TYPE::REP_ID repId);

		void BackendUpdate() override;

		void OnConnect() override;
		void OnConnectionConfirmed() override;
		void OnDisconnect() override;
		void OnUpdate() override;

		void OnReloadDenied() override;
		void OnReloadConfirmed() override;

		void OnCreateEntity() override;
		void OnDestroyEntity() override;
		void OnConstructRegistry() override;

		void OnMoveUpdate() override;

		void OnComponentUpdate() override;
		void OnRPC() override;

		void OnEvent() override;

		void OnChatMessage() override;

		void OnPing() override;
		void OnBadPacket() override;

		std::queue<Nexus::Packet> onLoadedQueue;
		std::string m_serverAdress;
		unsigned short m_serverPort = 0;
		float m_timer = 0;

		bool m_requestReload = false;
	};
}
