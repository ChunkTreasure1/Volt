#include "circuitpch.h"
#include "Widgets/Widget.h"

#include "CircuitPainter.h"

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

namespace Circuit
{

	void Widget::BuildBaseArgs(const CircuitBaseArgs& baseArgs)
	{
		//m_LocalXPosition = baseArgs._X;
		//m_LocalYPosition = baseArgs._Y;
	}

	void Widget::OnPaint(CircuitPainter& painter)
	{
	}

	void Widget::CalculateBounds()
	{
		VT_PROFILE_FUNCTION();
		CircuitPainter painter;

		OnPaint(painter);

		std::vector<CircuitDrawCommand> commands = painter.GetCommands();

		Volt::Rect bounds = Volt::Rect(glm::vec2(m_LocalXPosition, m_LocalYPosition ), glm::vec2(0.f));

		for (CircuitDrawCommand& command : commands)
		{
			switch (command.type)
			{
			case CircuitPrimitiveType::Rect:
				bounds.MergeRectIntoThis(Volt::Rect(command.pixelPos, command.radiusHalfSize*2.f));
				break;

			case CircuitPrimitiveType::Circle:
				bounds.MergeRectIntoThis(Volt::Rect(command.pixelPos - glm::vec2(command.radiusHalfSize.x), command.radiusHalfSize.x * 2.f));
				break;
			case CircuitPrimitiveType::TextCharacter:
				bounds.MergeRectIntoThis(Volt::Rect(command.minMaxPx.x, command.minMaxPx.y, glm::abs(command.minMaxPx.z - command.minMaxPx.x), glm::abs(command.minMaxPx.w - command.minMaxPx.y)));
				break;
			}
		}

		std::pair<float, float> windowPos = Volt::WindowManager::Get().GetMainWindow().GetPosition();
		bounds.SetPosition(bounds.GetPosition());
		m_bounds = bounds;
	}

	Volt::Rect Widget::GetBounds()
	{
		return m_bounds;
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
