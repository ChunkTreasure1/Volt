#pragma once
#include "Circuit/Widgets/BuildingSyntaxUtility.h"
#include "Circuit/Widgets/Widget.h"
#include "Circuit/CircuitColor.h"

namespace Circuit
{
	class RectWidget : public Widget
	{
	public:
		CIRCUIT_API RectWidget() {};
		~RectWidget() {};

		CIRCUIT_BEGIN_ARGS(RectWidget)
			: _Color(255, 255, 255, 255)
			, _Width(100)
			, _Height(100)
		{
		}
		CIRCUIT_ARGUMENT(CircuitColor, Color);
		CIRCUIT_ARGUMENT(float, Width);
		CIRCUIT_ARGUMENT(float, Height);
		CIRCUIT_END_ARGS();

		CIRCUIT_API void Build(Arguments args);

		CIRCUIT_API void SetWidth(float width);
		CIRCUIT_API float GetWidth() const;

		CIRCUIT_API void SetHeight(float height);
		CIRCUIT_API float GetHeight() const;

		CIRCUIT_API void SetColor(CircuitColor color);
		CIRCUIT_API CircuitColor GetColor() const;

	private:
		float m_Width;
		float m_Height;
		CircuitColor m_Color;
	};
}
