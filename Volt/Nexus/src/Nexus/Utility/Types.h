#pragma once
#include <stdint.h>
#include <memory>
#include <Volt/Core/UUID.h>

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

		using REP_ID = uint16_t;
		using CLIENT_ID = uint16_t;

		using RPC_ID = uint16_t;

		inline static constexpr CLIENT_ID NIL_CLIENT_ID = 0;
		inline static const REP_ID NIL_REP_ID = REP_ID(0);

	}

	TYPE::CLIENT_ID RandClientID();
	TYPE::REP_ID RandRepID();
	TYPE::REP_ID RandRPCID();


	enum class eInternalErrorCode : uint8_t
	{
		NIL = 0,
	};
}
