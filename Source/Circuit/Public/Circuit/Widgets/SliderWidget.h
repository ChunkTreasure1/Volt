#pragma once
#include "Circuit/Widgets/Widget.h"

#include <EventSystem/EventListener.h>

class WidgetBuilder;

template <typename T>
class Observer;

namespace Volt
{
	class MouseMovedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}

namespace Circuit
{
	class CIRCUIT_API SliderWidget : public Widget, public Volt::EventListener
	{
	public:
		SliderWidget();
		virtual ~SliderWidget();

		CIRCUIT_BEGIN_ARGS(SliderWidget)
		{
		};
		CIRCUIT_ARGUMENT(float, Value);
		CIRCUIT_ARGUMENT(float, Min);
		CIRCUIT_ARGUMENT(float, Max);
		CIRCUIT_END_ARGS();

		void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;

		float GetValue() const;
		void SetValue(float value);

		float GetMinValue() const;
		void SetMinValue(float value);

		float GetMaxValue() const;
		void SetMaxValue(float value);

		float GetValueNormalized();
	private:
		void RegisterEventListeners();

		bool OnMouseMoved(Volt::MouseMovedEvent& e);
		bool OnMouseButtonPressed(Volt::MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(Volt::MouseButtonReleasedEvent& e);

		bool IsMouseInsideRect();

		void SetValueAccordingToMousePos();

		bool m_dragging;

		float m_Value;
		//unsigned int m_OnValueChangeHandle;
		float m_MinValue;
		float m_MaxValue;

		static constexpr uint32_t s_sliderWidth = 200;
		static constexpr uint32_t s_sliderHeight = 10;
		static constexpr uint32_t s_sliderHandleRadius = 10;
	};
}
