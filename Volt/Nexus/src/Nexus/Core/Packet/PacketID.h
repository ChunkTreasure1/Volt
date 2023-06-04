#pragma once

namespace Nexus
{
	using PacketIdType = uint16_t;
	enum class ePacketID : PacketIdType
	{
		NIL,

		CONNECT,
		CONNECTION_CONFIRMED,
		DISCONNECT,
		DISCONNECTION_CONFIRMED,

		CREATE_ENTITY,
		REMOVE_ENTITY,

		UPDATE,

		MOVE,

		COMPONENT_UPDATE,
		RPC,
		EVENT,

		CHAT_MESSAGE,
		PING,

		CLOSE // Last
	};
}
