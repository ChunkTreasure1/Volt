#include "circuitpch.h"
#include "LayoutWidget.h"
#include "Circuit/CircuitPainter.h"

namespace Circuit
{
	void LayoutWidget::Build(const Arguments& args)
	{
		m_orientation = args._Orientation;
	}
	void LayoutWidget::OnPaint(CircuitPainter& painter)
	{
		const Volt::Rect& basePainterArea = painter.GetAllotedArea();

		glm::vec2 offsetPosition = { 0,0 };
		for (Slice& slice : m_slices)
		{
			glm::vec2 sizeOffset = { 0,0 };
			if (slice.size < 0)
			{
				if (m_orientation == LayoutOrientation::Horizontal)
				{
					sizeOffset.x = slice.widget->GetBounds().GetSize().x;
				}
				else if (m_orientation == LayoutOrientation::Vertical)
				{
					sizeOffset.y = slice.widget->GetBounds().GetSize().y;
				}
			}
			else
			{
				if (m_orientation == LayoutOrientation::Horizontal)
				{
					sizeOffset.x = slice.size;
				}
				else if (m_orientation == LayoutOrientation::Vertical)
				{
					sizeOffset.y = slice.size;
				}
			}

			const glm::vec2 position = basePainterArea.GetPosition() + offsetPosition;
			const glm::vec2 size = basePainterArea.GetSize() + sizeOffset;

			offsetPosition += size;

			CircuitPainter subPainter = painter.CreateSubPainter(position, size);
			slice.widget->OnPaint(subPainter);
		}
	}
	void LayoutWidget::AddFixedSlice(Ref<Widget> contentWidget, float size)
	{
		VT_PROFILE_FUNCTION();
		Slice& newSlice = m_slices.emplace_back();
		newSlice.widget = contentWidget;
		newSlice.size = static_cast<float>(size);
	}

	void LayoutWidget::AddFlexibleSlice(Ref<Widget> contentWidget)
	{
		Slice& newSlice = m_slices.emplace_back();
		newSlice.widget = contentWidget;
	}


}
