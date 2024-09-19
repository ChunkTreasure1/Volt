#pragma once

#include "Circuit/Widgets/Widget.h"

#include <EventSystem/EventListener.h> 

namespace Volt
{
	class MouseMovedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}
namespace Circuit
{
	class CIRCUIT_API ButtonWidget : public Widget, public Volt::EventListener
	{
	public:
		ButtonWidget();
		virtual ~ButtonWidget();

		CIRCUIT_BEGIN_ARGS(ButtonWidget)
		{
		};
		CIRCUIT_END_ARGS();

		void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;

	private:
		void RegisterEventListeners();

		bool OnMouseMoved(Volt::MouseMovedEvent& e);
		bool OnMouseButtonPressed(Volt::MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(Volt::MouseButtonReleasedEvent& e);


		bool m_hovered;
		bool m_pressed;
	};
}
