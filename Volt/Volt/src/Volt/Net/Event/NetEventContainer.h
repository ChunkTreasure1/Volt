#pragma once
#include <queue>
#include "Volt/Net/Event/NetEvent.h"

namespace Volt
{
	class NetEventContainer
	{
	public:
		void AddIncomming(const NetEvent& in_event);
		bool GetIncomming(NetEvent& out_event);

		void AddOutgoing(const NetEvent& in_event);
		bool GetOutgoing(NetEvent& out_event);

		std::queue<NetEvent>& GetQueueIn();
		std::queue<NetEvent>& GetQueueOut();

		static void StaticAddIncomming(const NetEvent& in_event);
		static void StaticAddOutgoing(const NetEvent& in_event);

	private:
		std::queue<NetEvent> m_queueIn;
		std::queue<NetEvent> m_queueOut;
	};
}
