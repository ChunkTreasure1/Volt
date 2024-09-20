#pragma once
#include "Circuit/CircuitDrawCommand.h"

#include <CoreUtilities/Core.h>

namespace Volt
{
	class Font;
}

namespace Circuit
{
	class CircuitPainter
	{
	public:
		CircuitPainter();
		~CircuitPainter() = default;

		void AddRect(float x, float y, float width, float height, CircuitColor color, float rotation = 0, float scale = 1);
		void AddCircle(float x, float y, float radius, CircuitColor color, float scale = 1);
		void AddText(float x, float y, const std::string& text, Ref<Volt::Font> font, float maxWidth, CircuitColor color, float scale = 1.f);

		std::vector<CircuitDrawCommand> GetCommands();
	private:
		std::vector<CircuitDrawCommand> m_drawCommands;
	};
}
