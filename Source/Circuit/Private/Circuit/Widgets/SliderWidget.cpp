#include "circuitpch.h"

#include "Widgets/SliderWidget.h"

#include "Circuit/WidgetBuilder.h"
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

		if (IsMouseInsideRect())
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

	bool SliderWidget::IsMouseInsideRect()
	{
		glm::vec2 mousePos = Volt::Input::GetMousePosition();
		glm::vec2 windowPos = {Volt::WindowManager::Get().GetMainWindow().GetPosition().first, Volt::WindowManager::Get().GetMainWindow().GetPosition().second};
		glm::vec2 topLeft = {GetX() + windowPos.x, GetY() + windowPos.y};
		glm::vec2 bottomRight = { topLeft.x + s_sliderWidth, topLeft.y + s_sliderHeight};

		return (mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
		mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y);
	}
	void SliderWidget::SetValueAccordingToMousePos()
	{
		glm::vec2 mousePos = Volt::Input::GetMousePosition();
		glm::vec2 windowPos = { Volt::WindowManager::Get().GetMainWindow().GetPosition().first, Volt::WindowManager::Get().GetMainWindow().GetPosition().second };
		glm::vec2 topLeft = { GetX() + windowPos.x, GetY() + windowPos.y };
		glm::vec2 bottomRight = { topLeft.x + s_sliderWidth, topLeft.y + s_sliderHeight };

		// Clamp the mouse position within the slider bounds
		float clampedMouseX = glm::clamp(mousePos.x, topLeft.x, bottomRight.x);

		// Calculate the normalized value (0 to 1)
		float valueNormalized = (clampedMouseX - topLeft.x) / (bottomRight.x - topLeft.x);

		SetValue(m_MinValue + valueNormalized * (m_MaxValue - m_MinValue));
	}
}
