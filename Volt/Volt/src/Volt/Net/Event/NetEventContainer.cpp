#include "vtpch.h"
#include "NetEventContainer.h"
#include "Volt/Core/Application.h"

namespace Volt
{
	void NetEventContainer::AddIncomming(const NetEvent& in_event)
	{
		m_queueIn.push(in_event);
	}

	bool NetEventContainer::GetIncomming(NetEvent& out_event)
	{
		if (m_queueIn.empty()) return false;

		out_event.SetEvent(m_queueIn.front().GetEvent());
		out_event.SetId(m_queueIn.front().GetId());
		out_event.SetData(m_queueIn.front().GetData());

		m_queueIn.pop();
		return true;
	}

	void NetEventContainer::AddOutgoing(const NetEvent& in_event)
	{
		m_queueOut.push(in_event);
	}

	bool NetEventContainer::GetOutgoing(NetEvent& out_event)
	{
		if (m_queueOut.empty()) return false;

		out_event.SetEvent(m_queueOut.front().GetEvent());
		out_event.SetId(m_queueOut.front().GetId());
		out_event.SetData(m_queueOut.front().GetData());

		m_queueOut.pop();
		return true;
	}

	std::queue<NetEvent>& NetEventContainer::GetQueueIn()
	{
		return m_queueIn;
	}

	std::queue<NetEvent>& NetEventContainer::GetQueueOut()
	{
		return m_queueOut;
	}

	void NetEventContainer::StaticAddIncomming(const NetEvent& in_event)
	{
		Application::Get().GetNetHandler().GetEventContainer().AddIncomming(in_event);
	}

	void NetEventContainer::StaticAddOutgoing(const NetEvent& in_event)
	{
		Application::Get().GetNetHandler().GetEventContainer().AddIncomming(in_event);
	}
}
