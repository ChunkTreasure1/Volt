#include "circuitpch.h"
#include "Widgets/ButtonWidget.h"

#include "CircuitPainter.h"

#include <InputModule/Input.h>
#include <InputModule/Events/MouseEvents.h>

Circuit::ButtonWidget::ButtonWidget()
{
	RegisterEventListeners();
}

Circuit::ButtonWidget::~ButtonWidget()
{
}

void Circuit::ButtonWidget::Build(const Arguments& args)
{
	m_hovered = false;
	m_pressed = false;
}

void Circuit::ButtonWidget::OnPaint(CircuitPainter& painter)
{
	const CircuitColor baseColor(255, 255, 255);
	const CircuitColor hoveredColor(200, 200, 200);
	const CircuitColor pressedColor(150, 150, 150);

	const CircuitColor* buttonColor = &baseColor;
	if (m_pressed)
	{
		buttonColor = &pressedColor;
	}
	else if (m_hovered)
	{
		buttonColor = &hoveredColor;
	}
	
	painter.AddRect(GetX(), GetY(), 50, 50, *buttonColor);
}

void Circuit::ButtonWidget::RegisterEventListeners()
{
	RegisterListener<Volt::MouseMovedEvent>(VT_BIND_EVENT_FN(ButtonWidget::OnMouseMoved));
	RegisterListener<Volt::MouseButtonPressedEvent>(VT_BIND_EVENT_FN(ButtonWidget::OnMouseButtonPressed));
	RegisterListener<Volt::MouseButtonReleasedEvent>(VT_BIND_EVENT_FN(ButtonWidget::OnMouseButtonReleased));
}

bool Circuit::ButtonWidget::OnMouseMoved(Volt::MouseMovedEvent& e)
{
	m_hovered = GetBounds().IsPointInside(Volt::Input::GetMousePosition());

	//if we leave the button while it is pressed, depress it
	if (m_pressed && !m_hovered)
	{
		m_pressed = false;
	}
	return false;
}

bool Circuit::ButtonWidget::OnMouseButtonPressed(Volt::MouseButtonPressedEvent& e)
{
	if (m_hovered)
	{
		m_pressed = true;
	}
	return false;
}

bool Circuit::ButtonWidget::OnMouseButtonReleased(Volt::MouseButtonReleasedEvent& e)
{
	if (m_hovered && m_pressed)
	{
		//execute the thing
		m_pressed = false;
	}

	return false;
}
