#pragma once
#include "Circuit/CircuitCoreDefines.h"
#include <cstdint>
class CircuitColor
{
public:
	union
	{
		uint32_t m_Hex;
		struct { uint8_t m_R, m_G, m_B, m_A; };
	};

	CIRCUIT_API explicit CircuitColor(uint32_t hex = 0xFFFFFFFF);
	CIRCUIT_API CircuitColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
};
