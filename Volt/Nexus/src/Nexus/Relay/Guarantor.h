#pragma once
#include <unordered_map>
#include "Nexus/Core/Types.h"
#include "Nexus/Core/Address.h"
#include "Nexus/Packet/Packet.h"
#include "Nexus/Utility/tsdeque.h"

namespace Nexus
{
	struct GuaranteeData
	{
		Nexus::Address target;
		Nexus::Packet packet;
		float time;
		uint8_t tries;
	};

	class Guarantor
	{
	public:
		Guarantor(tsdeque<std::pair<Nexus::Address, Packet>>& _q, std::atomic<bool>& running);
		~Guarantor();

		// own thread, managed by relay
		void Update();
		void Clear();

		Nexus::TYPE::NOTIFY_ID Add(const Nexus::Address& _address, const Nexus::Packet& _packet);
		bool Handle(const Packet& _packet, const Nexus::Address& _address);

	private:
		std::atomic<bool>& ex_isRunning;
		tsdeque<std::pair<Nexus::Address, Nexus::Packet>>& ex_outQue;

		std::unordered_map<TYPE::NOTIFY_ID, GuaranteeData> m_map;
		std::unordered_map<TYPE::NOTIFY_ID, int> m_handled;
		uint64_t m_droppedPacketCount = 0;
		// #nexus_todo: need thread safing
		// #nexus_todo: need time management
	};
}
