#pragma once
#include <vector>

#include "Nexus/Utility/Types.h"
#include "PacketID.h"
#include "Nexus/Core/Defines.h"
#include "Nexus/Utility/Log/Log.h"

namespace Nexus
{

	// #nexus_todo: failsafe unloading functions
	struct Packet
	{
		TYPE::CLIENT_ID ownerID = 0;
		ePacketID id = ePacketID::NIL;
		std::vector<TYPE::BYTE> body;

		size_t Size() const { return sizeof(ePacketID) + sizeof(TYPE::CLIENT_ID) + body.size(); }
		bool TooBig(size_t amount) { return (Size() + amount) > PACKET_SIZE ? true : false; }

		std::vector<char> WGet()
		{
			std::vector<char> rVec;
			rVec.resize(Size());
			memcpy(rVec.data(), &ownerID, sizeof(ePacketID));
			memcpy(rVec.data() + sizeof(TYPE::CLIENT_ID), &id, sizeof(ePacketID));
			memcpy(rVec.data() + sizeof(ePacketID) + sizeof(TYPE::CLIENT_ID), body.data(), body.size());
			return rVec;
		}

		void Append(const void* data, size_t size)
		{
			if (TooBig(size))
			{
				LogError("Packet too big: " + std::to_string(Size() + size));
				// #nexus_todo: automatic packet split?
				return;
			}

			size_t i = body.size();
			body.resize(body.size() + size);
			memcpy(body.data() + i, data, size);
		}

		friend Packet& operator<<(Packet& packet, const Packet& data)
		{
			if (packet.id != data.id)
			{
				LogError("id mismatch in packet, ");
				return packet;
			}
			auto dataSize = data.body.size();
			auto packetSize = packet.body.size();

			if (packet.TooBig(dataSize))
			{
				LogError("Packet too big: " + std::to_string(packet.Size() + dataSize));
				// #nexus_todo: automatic packet split?
				return packet;
			}

			size_t i = dataSize;
			packet.body.resize(packetSize + dataSize);
			packet.Append((uint8_t*)data.body.data(), dataSize);
			return packet;
		}

		template<typename DATA_TYPE>
		friend Packet& operator<<(Packet& packet, const DATA_TYPE& data)
		{
			static_assert(std::is_trivially_copyable<DATA_TYPE>::value, "DATA_TYPE cant safely be memcopied");

			if (packet.TooBig(sizeof(DATA_TYPE)))
			{
				LogError("Packet too big: " + std::to_string(packet.Size() + sizeof(DATA_TYPE)));
				// #nexus_todo: automatic packet split?
				return packet;
			}

			size_t i = packet.body.size();
			packet.body.resize(packet.body.size() + sizeof(DATA_TYPE));
			memcpy(packet.body.data() + i, &data, sizeof(DATA_TYPE));
			return packet;
		}

		template<typename DATA_TYPE>
		friend Packet& operator>>(Packet& packet, DATA_TYPE& data)
		{
			static_assert(std::is_trivially_copyable<DATA_TYPE>::value, "DATA_TYPE cant safely be memcopied");

			size_t i = packet.body.size() - sizeof(DATA_TYPE);
			memcpy(&data, packet.body.data() + i, sizeof(DATA_TYPE));
			packet.body.resize(i);
			return packet;
		}

		friend Packet& operator<<(Packet& packet, const std::string& data)
		{
			if (packet.TooBig(data.size()))
			{
				LogError("Packet too big: " + std::to_string(packet.Size() + sizeof(data.size())));
				// #nexus_todo: automatic packet split?
				return packet;
			}

			size_t i = packet.body.size();
			packet.body.resize(packet.body.size() + data.size());
			memcpy(packet.body.data() + i, data.data(), data.size());
			//packet << data.size();
			return packet;
		}
		// Note: Requires size of data to be set before usage
		friend Packet& operator>>(Packet& packet, std::string& data)
		{
			data = packet.GetString((int32_t)data.size());
			return packet;
		}
		[[nodiscard]] std::string GetString(int len = 0)
		{
			if (len == 0) len = (int32_t)body.size();
			std::string data;
			if (len > body.size())
			{
				// Log error
				return data;
			}
			data.resize(len);
			memcpy_s(data.data(), data.size(), body.data() + body.size() - len, len);
			body.resize(body.size() - len);
			return data;
		}
	};

	inline Packet ConstructPacket(char* in_buffer, int in_len)
	{
		auto clientIdSize = sizeof(TYPE::CLIENT_ID);
		auto idSize = sizeof(ePacketID);
		auto bodySize = in_len - idSize - clientIdSize;

		Packet constructed;
		memcpy_s(&constructed.ownerID, clientIdSize, &in_buffer[0], clientIdSize);
		memcpy_s(&constructed.id, idSize, &in_buffer[clientIdSize], idSize);

		if (bodySize <= 0 || bodySize >= PACKET_SIZE) return constructed;

		constructed.body.resize(bodySize);
		// #nexus_todo: Failsafe len / buffer size
		memcpy_s(constructed.body.data(), bodySize, &in_buffer[clientIdSize + idSize], bodySize);
		return constructed;
	}
}