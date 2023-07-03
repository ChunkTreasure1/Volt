#pragma once
#include <stdint.h>
#include <memory>

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

		using REP_ID = uint64_t;
		inline static const REP_ID NIL_REP_ID = REP_ID(0);

		using  CLIENT_ID = uint16_t;
		inline static constexpr CLIENT_ID NIL_CLIENT_ID = 0;

		using  RPC_ID = uint16_t;
		inline static constexpr RPC_ID NIL_RPC_ID = 0;

		using NETSCENE_INSTANCE_ID = uint16_t;
		inline static constexpr NETSCENE_INSTANCE_ID NIL_NETSCENE_INSTANCE_ID = 0;

		//using  NOTIFY_ID = uint64_t;
		//inline static constexpr NOTIFY_ID NIL_NOTIFY_ID = 0;
	}

	TYPE::CLIENT_ID RandClientID();
	TYPE::REP_ID RandRepID();
	TYPE::RPC_ID RandRPCID();
	//TYPE::NOTIFY_ID RandNotifyID();
	TYPE::NETSCENE_INSTANCE_ID RandSceneInstanceID();


	enum class eInternalErrorCode : uint8_t
	{
		NIL = 0,
	};
}
