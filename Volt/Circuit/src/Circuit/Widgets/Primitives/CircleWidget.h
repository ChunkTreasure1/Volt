#pragma once
#include "Circuit/Widgets/BuildingSyntaxUtility.h"
#include "Circuit/Widgets/Widget.h"
#include "Circuit/CircuitColor.h"

class CircuitColor;

class CircleWidget : public Widget
{
public:
	CIRCUIT_API CircleWidget() {};
	~CircleWidget() {};

	CIRCUIT_BEGIN_ARGS(CircleWidget)
		: _Color(255, 255, 255, 255)
		, _Radius(10)
	{
	}
	CIRCUIT_ARGUMENT(CircuitColor, Color);
	CIRCUIT_ARGUMENT(float, Radius);
	CIRCUIT_END_ARGS();

	CIRCUIT_API void Build(Arguments args);

	CIRCUIT_API void SetRadius(float radius);
	CIRCUIT_API float GetRadius() const;
	
	CIRCUIT_API void SetColor(CircuitColor color);
	CIRCUIT_API CircuitColor GetColor() const;

private:
	float m_Radius;
	CircuitColor m_Color;
};
