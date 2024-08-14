#include "sbpch.h"
#include "NetPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/UserSettingsManager.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Events/ApplicationEvents.h>

#include "Volt/Net/Serialization/NetSerialization.h"

#include <Nexus/Winsock/AddressHelpers.hpp>
#include <Volt/Net/Client/NetClient.h>

NetPanel::NetPanel()
	: EditorWindow("Network Panel")
{
}

NetPanel::~NetPanel()
{
}

void NetPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(NetPanel::OnUpdateEvent));
}

void NetPanel::UpdateMainContent()
{
	DrawPanel();
}

bool NetPanel::OnUpdateEvent(Volt::Event& e)
{
	e;
	return false;
}

void NetPanel::DrawPanel()
{
	float statRegionSize = ImGui::GetContentRegionAvail().y * m_panelSettings.statsPortion;
	ImGui::BeginChild("Stats##NetPanelStats", { 0, statRegionSize }, true);
	{
		DrawStats();
	}ImGui::EndChild();

	ImGui::BeginChild("General##NetPanelGeneral", { 0, 0 }, false);
	{
		ImGui::Checkbox("Enable Networking", &UserSettingsManager::GetSettings().networkSettings.enableNetworking); // THIS CURRENTLY DOES NOTHING BUT COULD BE USEFUL AND SAVES TO USERSETTINGS
		DrawDebug();
		DrawVirtualPacketConstructor();
	}ImGui::EndChild();

	DrawPopups();
}

void NetPanel::DrawStats()
{
	UI::Header("Stats");
	auto regionAvail = ImGui::GetContentRegionAvail();
	ImGui::SameLine();
	if (ImGui::Button("##NetPanelClearBtn", { regionAvail.x, 0 }))
	{

	}
	ImGui::Separator();

	if (Nexus::WSA::Session::IsValid())		ImGui::TextColored(ImColor(0, 255, 0), "Valid WSA Session");
	else									ImGui::TextColored(ImColor(255, 0, 0), "Invalid WSA Session");
	ImGui::Separator();

	if (!Nexus::WSA::Session::IsValid()) return;

	if (ImGui::BeginTable("##NetPanelStatTable", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Server Stats", 0, ImGui::GetContentRegionAvail().x * 0.5f);
		ImGui::TableSetupColumn("Client Stats", 0, ImGui::GetContentRegionAvail().x * 0.5f);

		// C1
		ImGui::TableNextColumn();

		ImGui::TextUnformatted("Server Stats");
		ImGui::SameLine();
		ImGui::Dummy({ 0, 20 });
		ImGui::Separator();
		{
			ImGui::TextUnformatted("ConnectionCount:");
			ImGui::SameLine();
			if (ImGui::Button(std::to_string(Volt::Application::Get().GetNetHandler().m_backend->GetConnectionRegistry().GetClientIDs().size()).c_str(), { ImGui::GetContentRegionAvail().x, 0 }))
			{
				UI::OpenPopup("Current connections");
			}
		}
		// C2
		ImGui::TableNextColumn();
		ImGui::TextUnformatted("Client Stats");
		ImGui::SameLine();
		ImGui::Dummy({ 0, 20 });
		ImGui::Separator();
		{

		}
		ImGui::EndTable();
	}

	// Tooltips
	auto hoveredItemId = ImGui::GetHoveredID();
	if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("##NetPanelClearBtn"))
	{
		ImGui::SetTooltip("Clear stats session");
	}

	/* #nexus_todo: list of stat support that need impl */

	///////////////////
	/*  Server stats
	* Connections:
	 - pings
	 - pps (packets per second)
	 -# avg packet loss
	* Handle time
	*
	*/

	///////////////////
	/*  Client stats
	* ping
	* pps (packets per second)
	* avg packet loss
	* handle time
	*/
}

void NetPanel::DrawDebug()
{
	if (ImGui::CollapsingHeader("Debug connections"))
	{
		ImGui::BeginChild("Stats##NetPanelDebugConnections", { 0, m_panelSettings.regionHeight }, true);

		auto& netHandler = Volt::Application::Get().GetNetHandler();

		auto regionAvail = ImGui::GetContentRegionAvail();
		// row 1
		if (ImGui::Button("Host##NetPanelHostBtn", { regionAvail.x * 0.5f, 0 }))
		{
			netHandler.SetForcedPort(m_netSettings.hostingPort);
			netHandler.StartServer();
		}
		ImGui::SameLine();
		if (ImGui::Button("Connect##NetPanelConnectBtn", { regionAvail.x * 0.5f, 0 }))
		{
			if (!m_netSettings.independentConnection)
			{
				netHandler.SetForcedPort(m_netSettings.hostingPort);
				netHandler.StartServer();
				//netHandler.m_backend->Start();
				//netHandler.m_backend->StoreServerData(m_netSettings.connectAddress, m_netSettings.hostingPort);

				Nexus::Packet connectionPacket;
				connectionPacket.id = Nexus::ePacketID::CONNECT;
				connectionPacket << std::string("Host Client");
				netHandler.m_backend->GetIncommingPacketQueue().push_back({ Nexus::CreateSockAddr("127.0.0.1", netHandler.m_backend->GetRelay().GetBoundPort()),connectionPacket });
			}
			else
			{
				netHandler.StartClient();
				((Volt::NetClient*)(netHandler.m_backend.get()))->StoreServerData(m_netSettings.connectAddress, m_netSettings.connectPort);
				Nexus::Packet connectionPacket;
				connectionPacket.id = Nexus::ePacketID::CONNECT;
				connectionPacket << std::string("Connected Client");
				netHandler.m_backend->Transmit(connectionPacket);
			}
		}

		// row 2		
		if (ImGui::Button("Cancel##NetPanelCancelBtn", { regionAvail.x, 0 }))
		{
			netHandler.Stop();
		}

		DrawHost();
		DrawConnect();

		// Tooltips
		auto hoveredItemId = ImGui::GetHoveredID();
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("Host##NetPanelHostBtn"))
		{
			ImGui::SetTooltip("");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("Connect##NetPanelConnectBtn"))
		{
			ImGui::SetTooltip("");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("Cancel##NetPanelCancelBtn"))
		{
			ImGui::SetTooltip("");
		}
		ImGui::EndChild();
	}
}

void NetPanel::DrawHost()
{
	if (UI::BeginProperties("NetPanelHostProperties"))
	{
		UI::Property("Host port", m_netSettings.hostingPort, "Default port: " + std::to_string(DEFAULT_PORT));

		UI::EndProperties();
	}
}

void NetPanel::DrawConnect()
{
	ImGui::Checkbox("##NetPanelConnectIndependantConnection", &m_netSettings.independentConnection);
	ImGui::SameLine();
	if (ImGui::CollapsingHeader("Independent Connection##NetPanelConnectHeader"))
	{
		if (UI::BeginProperties("NetPanelConnectProperties"))
		{
			UI::Property("   IPv4", m_netSettings.connectAddress, !m_netSettings.independentConnection, "IPv4 format\nDefault address: local host(" + std::string(LOCAL_HOST) + ")");
			UI::Property("   Port", m_netSettings.connectPort, "Default port: " + std::to_string(DEFAULT_PORT));
			if (!m_netSettings.independentConnection) m_netSettings.connectPort = m_netSettings.hostingPort;

			UI::EndProperties();
		}
	}
}

void NetPanel::DrawPopups()
{
	if (UI::BeginPopup("Current connections"))
	{
		for (auto id : Volt::Application::Get().GetNetHandler().m_backend->GetConnectionRegistry().GetClientIDs())
		{
			ImGui::LabelText(Volt::Application::Get().GetNetHandler().m_backend->GetConnectionRegistry().GetAlias(id.first).c_str(), std::to_string(id.first).c_str());
			ImGui::Separator();
		}
		UI::EndPopup();
	}
}

void NetPanel::DrawEventDebugger()
{

}

void NetPanel::DrawVirtualPacketConstructor()
{
	if (ImGui::CollapsingHeader("Virtual Packet##NetPanelVirtualPacketHeader"))
	{
		ImGui::BeginChild("##NetPanelVirtualPacketRegion", { 0, m_panelSettings.regionHeight }, true);

		auto regionAvail = ImGui::GetContentRegionAvail();
		if (ImGui::Button("Send As Server##NetPanelVPackSendAsServer", { regionAvail.x * 0.5f, 0 }))
		{
			auto p = ConstructPacket();
		}
		ImGui::SameLine();
		if (ImGui::Button("Send As Client##NetPanelVPackSendAsClient", { regionAvail.x * 0.5f, 0 }))
		{
			auto p = ConstructPacket();
			Volt::Application::Get().GetNetHandler().m_backend->Transmit(p);
		}

		if (UI::BeginProperties("NetPanelConnectProperties"))
		{
			UI::Property("Packet ID", *(Nexus::PacketIdType*)&m_virtualPacketSettings.packetId);
			if ((Nexus::PacketIdType)m_virtualPacketSettings.packetId > (Nexus::PacketIdType)Nexus::ePacketID::PING)
				m_virtualPacketSettings.packetId = Nexus::ePacketID::PING;
			UI::Property("Data", m_virtualPacketSettings.data);
			UI::EndProperties();
		}

		// Tooltips
		auto hoveredItemId = ImGui::GetHoveredID();
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("Send As Server##NetPanelVPackSendAsServer"))
		{
			ImGui::SetTooltip("");
		}
		if (hoveredItemId == ImGui::GetCurrentWindow()->GetID("Send As Client##NetPanelVPackSendAsClient"))
		{
			ImGui::SetTooltip("");
		}
		ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Virtual RPC Packet##NetPanelVirtualRPCPacketHeader"))
	{
		ImGui::BeginChild("##NetPanelVirtualPacketRPCRegion", { 0, m_panelSettings.regionHeight }, true);

		auto regionAvail = ImGui::GetContentRegionAvail();
		if (ImGui::Button("Send As Server##NetPanelVRPCPackSendAsServer", { regionAvail.x * 0.5f, 0 }))
		{
			auto p = ConstructRPCPacket();
			p.id = Nexus::ePacketID::RPC;
			for (const auto& _pair : Volt::Application::Get().GetNetHandler().m_backend->GetConnectionRegistry().GetClientIDs())
			{
				Volt::Application::Get().GetNetHandler().m_backend->GetRelay().Transmit(p, _pair.second);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Send As Client##NetPanelVRPCPackSendAsClient", { regionAvail.x * 0.5f, 0 }))
		{
			auto p = ConstructRPCPacket();
			Volt::Application::Get().GetNetHandler().m_backend->Transmit(p);
		}

		if (UI::BeginProperties("NetPanelConnectProperties"))
		{
			//UI::Property("Rep ID", m_RPCPacketSettings.repId);
			UI::Property("Project", m_RPCPacketSettings.monoProject);
			UI::Property("Class", m_RPCPacketSettings.monoClass);
			UI::Property("Method", m_RPCPacketSettings.monoMethod);
			UI::EndProperties();
		}

		ImGui::EndChild();
	}
	if (ImGui::CollapsingHeader("Virtual Event Packet##NetPanelVirtualRPCPacketHeaderEvent"))
	{
		ImGui::BeginChild("##NetPanelVirtualPacketRPCRegionEvent", { 0, m_panelSettings.regionHeight }, true);

		auto regionAvail = ImGui::GetContentRegionAvail();
		if (ImGui::Button("Send##NetPanelVRPCPackSendAsServer", { regionAvail.x, 0 }))
		{
			Volt::NetEvent netEvent;
			netEvent.m_event = m_eventSetting.netEvent;
			netEvent.m_id = m_eventSetting.repId;
			Volt::NetEventContainer::StaticAddIncomming(netEvent);
			Volt::NetEventContainer::StaticAddOutgoing(netEvent);
		}
		if (UI::BeginProperties("NetPanelConnectProperties"))
		{
			//UI::Property("Rep ID", m_eventSetting.repId);
			static int addEvent = 0;
			
			{
				const Volt::IEnumTypeDesc* netEnumType = reinterpret_cast<const Volt::IEnumTypeDesc*>(Volt::GetTypeDesc<Volt::eNetEvent>());

				int32_t currentIndex = 0;

				std::unordered_map<int32_t, int32_t> indexToValueMap;
				Vector<std::string> constantNames;

				for (uint32_t index = 0; const auto & constant : netEnumType->GetConstants())
				{
					const std::string name = std::string(constant.label);
					constantNames.emplace_back(name);
					indexToValueMap[index] = constant.value;

					if (constant.value == addEvent)
					{
						currentIndex = index;
					}

					index++;
				}

				if (UI::ComboProperty("Manage Calls", addEvent, constantNames))
				{
					addEvent = indexToValueMap.at(addEvent);
				}
			}

			m_eventSetting.netEvent = static_cast<Volt::eNetEvent>(addEvent);
			UI::EndProperties();
		}
		if (ImGui::Button("Internal Debug##netpanelinternaldebugeventbtn", { regionAvail.x, 0 }))
		{
			Volt::NetEvent eventIn(Volt::eNetEvent::Hit, 10, Vector<uint8_t>({ 1, 2, 23, 11, 4, 1, 2, 2, 9, 2, 9 }));
			Nexus::Packet ePac = SerializeNetEventPacket(eventIn);
			Volt::NetEvent eventOut;
			ePac > eventOut;
		}
		ImGui::EndChild();
	}
}

Nexus::Packet NetPanel::ConstructPacket()
{
	Nexus::Packet packet;
	packet.id = static_cast<Nexus::ePacketID>(m_virtualPacketSettings.packetId);
	packet << m_virtualPacketSettings.data;
	return packet;
}

Nexus::Packet NetPanel::ConstructRPCPacket()
{
	Nexus::Packet packet = SerializeRPCPacket(m_RPCPacketSettings.monoProject, m_RPCPacketSettings.monoClass, m_RPCPacketSettings.monoMethod, m_RPCPacketSettings.repId);
	return packet;
}
