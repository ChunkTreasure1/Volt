#pragma once
#include <stdint.h>
#include <memory>
#include "Nexus/Packet/PacketId.h"
#include "Nexus/API/CONFIG.h"

namespace Nexus
{
	namespace TYPE
	{
		using BYTE = uint8_t;
		enum class eReplicatedType : uint8_t
		{
			NIL = 0,
			ENTITY,
			VARIABLE,
			COMPONENT
		};

		// Scene
		using RPC_ID = NXS_TYPE_RPC_ID;
		using REP_ID = NXS_TYPE_REP_ID;
		using REP_SUB_ID = NXS_TYPE_SUB_ID;
		using INSTANCE_ID = NXS_TYPE_INSTANCE;

		// Net backend
		using CLIENT_ID = NXS_TYPE_CLIENT_ID;
		using NOTIFY_ID = NXS_TYPE_NOTIFY_ID;

		inline static constexpr CLIENT_ID NIL_CLIENT_ID = 0;
		inline static const REP_ID NIL_REP_ID = REP_ID(0);
	}
}
