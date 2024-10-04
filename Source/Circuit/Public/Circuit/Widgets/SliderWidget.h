#pragma once
#include "Circuit/Widgets/Widget.h"

#include <CoreUtilities/Delegates/Delegate.h>

#include <EventSystem/EventListener.h>


namespace Volt
{
	class MouseMovedEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
}

DECLARE_DELEGATE_OneParam(OnFloatValueChangedDelegate, float /*NewValue*/);
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

		CIRCUIT_ATTRIBUTE(float, Value);

		CIRCUIT_ARGUMENT(float, MinValue);
		CIRCUIT_ARGUMENT(float, MaxValue);

		CIRCUIT_EVENT(OnFloatValueChangedDelegate, OnValueChanged)
		CIRCUIT_END_ARGS();

		void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;

		float GetValue() const;

		float GetMinValue() const;

		float GetMaxValue() const;

		float GetValueNormalized();
	private:
		void RegisterEventListeners();

		bool OnMouseMoved(Volt::MouseMovedEvent& e);
		bool OnMouseButtonPressed(Volt::MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(Volt::MouseButtonReleasedEvent& e);

		void SetValueAccordingToMousePos();

		bool m_dragging;

		Volt::Attribute<float> m_value;

		float m_minValue;
		float m_maxValue;

		OnFloatValueChangedDelegate m_onValueChanged;

		static constexpr uint32_t s_sliderWidth = 200;
		static constexpr uint32_t s_sliderHeight = 10;
		static constexpr uint32_t s_sliderHandleRadius = 10;
	};
}
