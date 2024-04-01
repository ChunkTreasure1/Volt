#include "circuitpch.h"
#include "CircuitColor.h"

CircuitColor::CircuitColor(uint32_t hex)
{
	m_Hex = hex;
}

CircuitColor::CircuitColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	m_R = r;
	m_G = g;
	m_B = b;
	m_A = a;
}
