#include "circuitpch.h"
#include "SliderWidget.h"
#include "Circuit/WidgetBuilder.h"
#include "Circuit/CircuitColor.h"
#include "Circuit/Widgets/Primitives/RectWidget.h"
#include "Circuit/Widgets/Primitives/CircleWidget.h"
#include "Delegates/Observer.h"

#include "Circuit/CircuitPainter.h"

namespace Circuit
{

	SliderWidget::SliderWidget()
	{
	}


	SliderWidget::~SliderWidget()
	{
		//m_Value->GetOnChangeDelegate().Remove(m_OnValueChangeHandle);

	}

	void SliderWidget::Build(const Arguments& args)
	{
		m_MinValue = args._Min;
		m_MaxValue = args._Max;
		m_Value = args._Value;
	}

	void SliderWidget::OnPaint(CircuitPainter& painter)
	{
		const uint32_t sliderWidth = 100;
		const uint32_t sliderHeight = 20;
		const float leftRectWidth = sliderWidth * GetValueNormalized();

		const CircuitColor unfilledColor = 0x555555ff;
		const CircuitColor filledColor = 0xf5f5f5ff;
		const CircuitColor handleColor = 0x000000ff;


		//left rect
		painter.AddRect(GetX(), GetY(), leftRectWidth, sliderHeight, filledColor);

		//right rect
		painter.AddRect(GetX() + leftRectWidth, GetY(), sliderWidth - leftRectWidth, sliderHeight, unfilledColor);

		//handle outer
		painter.AddCircle(GetX() + leftRectWidth, GetY() + sliderHeight/2, (sliderHeight / 2) * 1.1f, handleColor);

		//handle inner
		painter.AddCircle(GetX() + leftRectWidth, GetY() + sliderHeight/2, (sliderHeight / 2) * 0.7f, filledColor);

	}

	float SliderWidget::GetValue() const
	{
		return m_Value;
	}

	void SliderWidget::SetValue(float value)
	{
		m_Value = value;
	}

	float SliderWidget::GetMinValue() const
	{
		return m_MinValue;
	}

	void SliderWidget::SetMinValue(float value)
	{
		m_MinValue = value;
	}


	float SliderWidget::GetMaxValue() const
	{
		return m_MaxValue;
	}

	void SliderWidget::SetMaxValue(float value)
	{
		m_MaxValue = value;
	}


	float SliderWidget::GetValueNormalized()
	{
		return (m_Value - m_MinValue) / (m_MaxValue - m_MinValue);
	}
}
