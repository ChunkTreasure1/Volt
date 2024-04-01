#include "circuitpch.h"
#include "CircleWidget.h"

void CircleWidget::Build(Arguments args)
{
	m_IsRenderPrimitive = true;
	m_RenderPrimitiveType = RenderPrimitiveType::Circle;

	m_Radius = args._Radius;
	m_Color = args._Color;
}

void CircleWidget::SetRadius(float radius)
{
	m_Radius = radius;
}

float CircleWidget::GetRadius() const
{
	return m_Radius;
}

void CircleWidget::SetColor(CircuitColor color)
{
	m_Color = color;
}

CircuitColor CircleWidget::GetColor() const
{
	return m_Color;
}
