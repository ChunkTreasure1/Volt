#pragma once

#include <cstdint>
#include <xhash>

// Based on CryEngines CryGUID
struct WireGUID
{
	constexpr WireGUID()
		: hiPart(0), loPart(0)
	{
	}

	constexpr WireGUID(const WireGUID& rhs)
		: hiPart(rhs.hiPart), loPart(rhs.loPart)
	{
	}

	constexpr WireGUID(const uint64_t& hi, const uint64_t& lo)
		: hiPart(hi), loPart(lo)
	{
	}

	constexpr static WireGUID Construct(uint32_t d1, uint16_t d2, uint16_t d3,
		uint8_t d4_0, uint8_t d4_1, uint8_t d4_2, uint8_t d4_3, uint8_t d4_4, uint8_t d4_5, uint8_t d4_6, uint8_t d4_7)
	{
		return WireGUID(
			(((uint64_t)d3) << 48) | (((uint64_t)d2) << 32) | ((uint64_t)d1),  //high part
			(((uint64_t)d4_7) << 56) | (((uint64_t)d4_6) << 48) | (((uint64_t)d4_5) << 40) | (((uint64_t)d4_4) << 32) | (((uint64_t)d4_3) << 24) | (((uint64_t)d4_2) << 16) | (((uint64_t)d4_1) << 8) | (uint64_t)d4_0   //low part
		);
	}

	constexpr static WireGUID Null()
	{
		return WireGUID(0, 0);
	}

	constexpr bool IsNull() const { return hiPart == 0 && loPart == 0; }
	constexpr bool operator==(const WireGUID& rhs) const { return hiPart == rhs.hiPart && loPart == rhs.loPart; }
	constexpr bool operator!=(const WireGUID& rhs) const { return hiPart != rhs.hiPart || loPart != rhs.loPart; }
	constexpr bool operator<(const WireGUID& rhs) const { return hiPart < rhs.hiPart || ((hiPart == rhs.hiPart) && loPart < rhs.loPart); }
	constexpr bool operator>(const WireGUID& rhs) const { return hiPart > rhs.hiPart || ((hiPart == rhs.hiPart) && loPart > rhs.loPart); }
	constexpr bool operator<=(const WireGUID& rhs) const { return hiPart < rhs.hiPart || ((hiPart == rhs.hiPart) && loPart <= rhs.loPart); }
	constexpr bool operator>=(const WireGUID& rhs) const { return hiPart > rhs.hiPart || ((hiPart == rhs.hiPart) && loPart >= rhs.loPart); }

	struct StringUtils
	{
		constexpr static uint8_t HexCharToUInt8(char x)
		{
			return x >= '0' && x <= '9' ? x - '0' :
				x >= 'a' && x <= 'f' ? x - 'a' + 10 :
				x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0;
		}

		constexpr static uint16_t HexCharToUInt16(char x)
		{
			return static_cast<uint16_t>(HexCharToUInt8(x));
		}

		constexpr static uint32_t HexCharToUInt32(char x)
		{
			return static_cast<uint32_t>(HexCharToUInt8(x));
		}

		constexpr static uint8_t HexStringToUInt8(const char* szInput)
		{
			return (HexCharToUInt8(szInput[0]) << 4) +
				HexCharToUInt8(szInput[1]);
		}

		constexpr static uint16_t HexStringToUInt16(const char* szInput)
		{
			return (HexCharToUInt16(szInput[0]) << 12) +
				(HexCharToUInt16(szInput[1]) << 8) +
				(HexCharToUInt16(szInput[2]) << 4) +
				HexCharToUInt16(szInput[3]);
		}

		constexpr static uint32_t HexStringToUInt32(const char* szInput)
		{
			return (HexCharToUInt32(szInput[0]) << 28) +
				(HexCharToUInt32(szInput[1]) << 24) +
				(HexCharToUInt32(szInput[2]) << 20) +
				(HexCharToUInt32(szInput[3]) << 16) +
				(HexCharToUInt32(szInput[4]) << 12) +
				(HexCharToUInt32(szInput[5]) << 8) +
				(HexCharToUInt32(szInput[6]) << 4) +
				HexCharToUInt32(szInput[7]);
		}
	};

	constexpr static WireGUID FromStringInternal(const char* string)
	{
		return WireGUID::Construct
		(
			StringUtils::HexStringToUInt32(string),
			StringUtils::HexStringToUInt16(string + 9),
			StringUtils::HexStringToUInt16(string + 14),
			StringUtils::HexStringToUInt8(string + 19),
			StringUtils::HexStringToUInt8(string + 21),
			StringUtils::HexStringToUInt8(string + 24),
			StringUtils::HexStringToUInt8(string + 26),
			StringUtils::HexStringToUInt8(string + 28),
			StringUtils::HexStringToUInt8(string + 30),
			StringUtils::HexStringToUInt8(string + 32),
			StringUtils::HexStringToUInt8(string + 34)
		);
	}

	uint64_t hiPart;
	uint64_t loPart;
};

constexpr WireGUID operator"" _guid(const char* input, size_t)
{
	return (input[0] == '{') ? WireGUID::FromStringInternal(input + 1) : WireGUID::FromStringInternal(input);
}

namespace std
{
	template<>
	struct hash<WireGUID>
	{
		size_t operator()(const WireGUID& guid) const
		{
			std::hash<uint64_t> hasher;
			return hasher(guid.loPart) ^ hasher(guid.hiPart);
		}
	};
}