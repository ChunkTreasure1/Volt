#pragma once

#include "Volt/Net/Event/NetEventContainer.h"
#include "Volt/Events/Event.h"

namespace Nexus
{
	class Manager;
}

namespace Volt
{
	enum class eNetErrorCode : uint8_t;

	class NetHandler
	{
	public:
		NetHandler();
		~NetHandler();

		void StartClient();
		void StartServer();
		void StartSinglePlayer();
		void Update(const float& deltaTime);
		void Stop();

		void SetForcedPort(unsigned short port) { m_forcedPort = port; }

		void Disconnect();

		bool IsRunning();
		eNetErrorCode* GetErrorPtr();
		NetEventContainer& GetEventContainer();
		Scope<Nexus::Manager>& GetBackend();

		bool IsOwner(Nexus::TYPE::REP_ID in_object);
		bool IsHost();

		void LoadNetScene();

		void OnEvent(Volt::Event& in_event);

		void EnableTick(bool tick) { m_handleTick = tick; }
		
		void Reload();

	private:
		friend class NetPanel;
		Scope<Nexus::Manager> m_backend;
		NetEventContainer m_eventContainer;
		eNetErrorCode* m_lastUnhandledNetError = nullptr;

		bool m_handleTick = true;
		bool m_isHost = false;
		bool m_netSceneLoaded = false;

		unsigned short m_forcedPort = 0;
	};
}
