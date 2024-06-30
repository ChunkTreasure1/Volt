#include "circuitpch.h"
#include "RectWidget.h"
namespace Circuit
{
	void RectWidget::Build(Arguments args)
	{
		m_IsRenderPrimitive = true;
		m_RenderPrimitiveType = RenderPrimitiveType::Rectangle;

		m_Width = args._Width;
		m_Height = args._Height;
		m_Color = args._Color;
	}

	void RectWidget::SetWidth(float width)
	{
		m_Width = width;
	}

	float RectWidget::GetWidth() const
	{
		return m_Width;
	}

	void RectWidget::SetHeight(float height)
	{
		m_Height = height;
	}

	float RectWidget::GetHeight() const
	{
		return m_Height;
	}

	void RectWidget::SetColor(CircuitColor color)
	{
		m_Color = color;
	}

	CircuitColor RectWidget::GetColor() const
	{
		return m_Color;
	}
}
