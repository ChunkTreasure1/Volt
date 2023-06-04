#pragma once
#include "Volt/Core/UUID.h"

namespace Volt::BehaviorTree
{
	struct Link
	{
		UUID m_parentID = 0;
		UUID m_childID = 0;
		UUID m_uuid = 0;

		bool operator==(const Link& in_link) { return (m_uuid == in_link.m_uuid); }
	};
	inline bool operator==(const Link& in_link1, const Link& in_link2) { return (in_link1.m_uuid == in_link2.m_uuid); }

	namespace Nil
	{
		inline static const Link link = { 0,0,0 };
		inline static const std::vector<Link> linkVec;
		inline static const UUID id = UUID(0);
	}

	enum class eNodeStatus : uint32_t
	{
		FAILURE = 0,
		//RUNNING,
		SUCCESS
	};

	const std::string decBfr[5]{
		"REPEAT_UNTIL_FAIL",
		"SUCCEEDER",
		"INVERTER",
		"REPEATER",
		"IF",
	};

	enum class eDecoratorType
	{
		REPEAT_UNTIL_FAIL,
		SUCCEEDER,
		INVERTER,
		REPEATER,
		IF,
	};
}
