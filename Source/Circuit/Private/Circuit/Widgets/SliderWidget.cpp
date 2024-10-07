#include "circuitpch.h"

#include "Widgets/SliderWidget.h"

#include "Circuit/CircuitColor.h"

#include "Circuit/CircuitPainter.h"

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <InputModule/Events/MouseEvents.h>
#include <InputModule/Input.h>

namespace Circuit
{

	SliderWidget::SliderWidget()
	{
		RegisterEventListeners();

		m_dragging = false;
	}


	SliderWidget::~SliderWidget()
	{
		//m_Value->GetOnChangeDelegate().Remove(m_OnValueChangeHandle);

	}

	void SliderWidget::Build(const Arguments& args)
	{
		m_minValue = args._MinValue;
		m_maxValue = args._MaxValue;

		m_value = args._Value;

		m_onValueChanged = args._OnValueChanged;
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

	float SliderWidget::GetValue() const
	{
		return m_value.Get();
	}

	float SliderWidget::GetMinValue() const
	{
		return m_minValue;
	}

	float SliderWidget::GetMaxValue() const
	{
		return m_maxValue;
	}

	float SliderWidget::GetValueNormalized()
	{
		return (m_value.Get() - m_minValue) / (m_maxValue - m_minValue);
	}

	void SliderWidget::RegisterEventListeners()
	{
		RegisterListener<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(SliderWidget::OnMouseMoved));
		RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(SliderWidget::OnMouseButtonPressed));
		RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(SliderWidget::OnMouseButtonReleased));
	}
	bool SliderWidget::OnMouseMoved(Volt::MouseMovedEvent& e)
	{
		if (!m_dragging)
		{
			return false;
		}

		SetValueAccordingToMousePos();

		return false;
	}
	bool SliderWidget::OnMouseButtonPressed(Volt::MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() != Volt::InputCode::Mouse_LB)
		{
			return false;
		}

		if (GetBounds().IsPointInside(Volt::Input::GetMousePosition()))
		{
			m_dragging = true;
			SetValueAccordingToMousePos();
		}

		return false;
	}
	bool SliderWidget::OnMouseButtonReleased(Volt::MouseButtonReleasedEvent& e)
	{
		if (e.GetMouseButton() != Volt::InputCode::Mouse_LB)
		{
			return false;
		}

		if (m_dragging)
		{
			m_dragging = false;
		}

		return false;
	}

	void SliderWidget::SetValueAccordingToMousePos()
	{
		const glm::vec2 mousePos = Volt::Input::GetMousePosition();
		float clamped = GetBounds().ClampInsideX(mousePos.x);

		const glm::vec2 topLeft = GetBounds().GetPosition();
		const glm::vec2 bottomRight = GetBounds().GetBottomRight();

		// Calculate the normalized value (0 to 1)
		float valueNormalized = (clamped - topLeft.x) / (bottomRight.x - topLeft.x);

		m_onValueChanged.ExecuteIfBound(m_minValue + valueNormalized * (m_maxValue - m_minValue));
	}
}
