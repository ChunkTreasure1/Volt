#include "circuitpch.h"
#include "CircuitPainter.h"

namespace Circuit
{

	CircuitPainter::CircuitPainter()
	{
	}

	void CircuitPainter::AddRect(float x, float y, float width, float height, CircuitColor color, float rotation, float scale)
	{
		CircuitDrawCommand command;
		command.type = CircuitPrimitiveType::Rect;

		command.pixelPos.x = x;
		command.pixelPos.y = y;

		command.radiusHalfSize.x = width / 2;
		command.radiusHalfSize.y = height / 2;

		command.rotation = rotation;

		command.scale = scale;

		//command.color = color;

		m_drawCommands.push_back(command);
	}

	void CircuitPainter::AddCircle(float x, float y, float radius, CircuitColor color, float scale)
	{
		CircuitDrawCommand command;
		command.type = CircuitPrimitiveType::Circle;
		command.pixelPos.x = x;
		command.pixelPos.y = y;

		command.radiusHalfSize.x = radius;

		command.scale = scale;

		//command.color = color;

		m_drawCommands.push_back(command);
	}

	std::vector<CircuitDrawCommand> CircuitPainter::GetCommands()
	{
		return m_drawCommands;
	}

}
