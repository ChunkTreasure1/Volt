#pragma once
#include "Sandbox/Window/EditorWindow.h"

#include <Nexus/Core/Defines.h>
#include <Nexus/Core/Packet/PacketID.h>
#include <Nexus/Core/Packet/Packet.hpp>

#include <Volt/Net/Event/NetEventContainer.h>

class NetPanel : public EditorWindow
{
public:
	NetPanel();
	~NetPanel();

	void OnEvent(Volt::Event& e) override;
	void UpdateMainContent() override;

private:
	struct NetSettings
	{
		bool independentConnection = false;
		unsigned short hostingPort = DEFAULT_PORT;
		unsigned short connectPort = DEFAULT_PORT;
		std::string connectAddress = LOCAL_HOST;
	} m_netSettings;

	struct VirutalPacketSettings
	{
		enum Nexus::ePacketID packetId = Nexus::ePacketID::PING;
		std::string data = "Ping";
	} m_virtualPacketSettings;

	struct RPCPacketSettings
	{
		Nexus::TYPE::REP_ID repId = 0;
		std::string monoProject = "Project";
		std::string monoClass = "NetMoveTest";
		std::string monoMethod = "RPCTest";
	} m_RPCPacketSettings;

	struct PanelSettings
	{
		float regionHeight = 125;
		float statsPortion = 0.4f;
	}m_panelSettings;

	struct EventSetting
	{
		Volt::eNetEvent netEvent = Volt::eNetEvent::NIL;
		Nexus::TYPE::REP_ID repId = 0;
	}m_eventSetting;

	bool OnUpdateEvent(Volt::Event& e);
	void DrawPanel();

	void DrawStats();

	void DrawDebug();
	void DrawHost();
	void DrawConnect();

	void DrawPopups();

	void DrawEventDebugger();

	void DrawVirtualPacketConstructor();
	Nexus::Packet ConstructPacket();
	Nexus::Packet ConstructRPCPacket();
};

