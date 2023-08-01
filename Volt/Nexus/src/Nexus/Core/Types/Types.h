#pragma once
#include <stdint.h>
#include <memory>
#include "Nexus/Core/Packet/PacketId.h"

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

		using RPC_ID = uint16_t;
		using REP_ID = uint16_t;
		using CLIENT_ID = uint16_t;
		using NOTIFY_ID = uint64_t;

		using NETSCENE_INSTANCE_ID = uint64_t;

		inline static constexpr CLIENT_ID NIL_CLIENT_ID = 0;
		inline static const REP_ID NIL_REP_ID = REP_ID(0);

		CLIENT_ID RandClientID();
		REP_ID RandRepID();
		RPC_ID RandRPCID();
		NOTIFY_ID RandNotifyID();
		NETSCENE_INSTANCE_ID RandSceneInstanceID();
	}

	enum class eInternalErrorCode : uint8_t
	{
		NIL = 0,
	};
}
