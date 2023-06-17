#pragma once

namespace Nexus
{
	// #nexus_was: uint16_t
	using PacketIdType = uint16_t;
	enum class ePacketID : PacketIdType
	{
		NIL,

		CONNECT,
		CONNECTION_DENIED,
		CONNECTION_CONFIRMED,

		DISCONNECT,
		DISCONNECTION_CONFIRMED,

		RELOAD,
		RELOAD_DENIED,
		RELOAD_CONFIRMED,

		CREATE_ENTITY,
		REMOVE_ENTITY,
		CONSTRUCT_REGISTRY,

		UPDATE,
		EVENT,
		RPC,
		CHAT_MESSAGE,

		// depricated
		MOVE,

		// editor
		COMPONENT_UPDATE,

		PING,

		CLOSE // Last
	};
}
