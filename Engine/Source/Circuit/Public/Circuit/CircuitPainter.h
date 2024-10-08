#pragma once
#include "Circuit/CircuitDrawCommand.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/Math/2DShapes/Rect.h>

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
		CircuitPainter(const Volt::Rect& allotedArea);
		CircuitPainter(const glm::vec2& position, const glm::vec2& size);

		CircuitPainter(CircuitPainter* basePainter, const Volt::Rect& allotedArea);
		CircuitPainter(CircuitPainter* basePainter, const glm::vec2& position, const glm::vec2& size);

		~CircuitPainter() = default;

		const Volt::Rect& GetAllotedArea() const;

		CircuitPainter CreateSubPainter(const Volt::Rect& allotedArea);
		CircuitPainter CreateSubPainter(const glm::vec2& position, const glm::vec2& size);

		void AddRect(float x, float y, float width, float height, CircuitColor color, float rotation = 0, float scale = 1);
		void AddCircle(float x, float y, float radius, CircuitColor color, float scale = 1);
		void AddText(float x, float y, const std::string& text, Ref<Volt::Font> font, float maxWidth, CircuitColor color, float scale = 1.f);

		std::vector<CircuitDrawCommand> GetCommands();
	private:
		std::vector<CircuitDrawCommand> m_drawCommands;

		Volt::Rect m_allottedArea;

		CircuitPainter* m_basePainter = nullptr;

	};
}
