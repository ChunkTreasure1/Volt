#include "circuitpch.h"

#include "Widgets/SliderWidget.h"

#include "Circuit/WidgetBuilder.h"
#include "Circuit/CircuitColor.h"

#include "Circuit/Input/CircuitInput.h"

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
		const float leftRectWidth = s_sliderWidth * GetValueNormalized();

		const CircuitColor unfilledColor = 0x555555ff;
		const CircuitColor filledColor = 0xffa500ff;
		const CircuitColor handleColor = 0x000000ff;


		//left rect
		painter.AddRect(GetX(), GetY(), leftRectWidth, s_sliderHeight, filledColor);

		//right rect
		painter.AddRect(GetX() + leftRectWidth, GetY(), s_sliderWidth - leftRectWidth, s_sliderHeight, unfilledColor);

		//handle outer
		painter.AddCircle(GetX() + leftRectWidth, GetY() + s_sliderHeight/2, s_sliderHandleRadius, handleColor);

		//handle inner
		painter.AddCircle(GetX() + leftRectWidth, GetY() + s_sliderHeight/2, (s_sliderHeight / 3), filledColor);

	}

	void SliderWidget::OnInputEvent(InputEvent& inputEvent)
	{
		if (inputEvent.GetKeyCode() != KeyCode::Mouse_LB)
		{
			return;
		}

		//if()

		if (inputEvent.WasJustPressed())
		{

		}
		else if (inputEvent.WasJustReleased())
		{

		}
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
