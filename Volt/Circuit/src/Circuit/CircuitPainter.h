#pragma once
#include "Circuit/CircuitDrawCommand.h"

namespace Circuit
{

	class CircuitPainter
	{
	public:
		CircuitPainter();
		~CircuitPainter() = default;

		void AddRect(float x, float y, float width, float height, CircuitColor color, float rotation = 0, float scale = 1);
		void AddCircle(float x, float y, float radius, CircuitColor color, float scale = 1);

		std::vector<CircuitDrawCommand> GetCommands();
	private:
		std::vector<CircuitDrawCommand> m_drawCommands;
	};
}
