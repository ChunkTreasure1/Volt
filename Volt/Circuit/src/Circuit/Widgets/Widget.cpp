#include "circuitpch.h"
#include "Widget.h"

namespace Circuit
{

	void Widget::BuildBaseArgs(const CircuitBaseArgs& baseArgs)
	{
		m_LocalXPosition = baseArgs._X;
		m_LocalYPosition = baseArgs._Y;
	}

	void Widget::OnPaint(CircuitPainter& painter)
	{
	}

	void Widget::RequestRebuild()
	{
		m_NeedsRebuild = true;
	}

	bool Widget::IsRenderPrimitive() const
	{
		return m_IsRenderPrimitive;
	}

	RenderPrimitiveType Widget::GetRenderPrimitiveType() const
	{
		return m_RenderPrimitiveType;
	}

}
