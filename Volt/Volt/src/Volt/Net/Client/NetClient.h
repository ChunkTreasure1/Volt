#pragma once
#include <Nexus/Interface/NetManager/NetManager.h>
namespace Volt
{
	class NetClient : public Nexus::NetManager
	{
	public:
		NetClient();
		~NetClient();

		void StoreServerData(std::string adress, unsigned short port) { m_serverAdress = adress; m_serverPort = port; }

		void Transmit(const Nexus::Packet& in_packet) override;
		void Init() override;
	private:
		friend class NetPanel;

		void MissingEntity(Nexus::TYPE::REP_ID repId);

		void Update() override;

		void OnConnect() override;
		void OnConnectionConfirmed() override;
		void OnDisconnect() override;
		void OnUpdate() override;

		void OnCreateEntity() override;
		void OnDestroyEntity() override;

		void OnMoveUpdate() override;

		void OnComponentUpdate() override;
		void OnRPC() override;

		void OnEvent() override;

		void OnChatMessage() override;

		void OnPing() override;
		void OnBadPacket() override;


		std::string m_serverAdress;
		unsigned short m_serverPort = 0;
		float m_timer = 0;
	};
}
