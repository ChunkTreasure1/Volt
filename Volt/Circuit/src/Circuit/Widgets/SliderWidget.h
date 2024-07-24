#pragma once
#include "Circuit/Widgets/Widget.h"

class WidgetBuilder;

template <typename T>
class Observer;
namespace Circuit
{
	class SliderWidget : public Widget
	{
	public:
		CIRCUIT_API SliderWidget();
		virtual ~SliderWidget();

		CIRCUIT_BEGIN_ARGS(SliderWidget)
		{
		};
		CIRCUIT_ARGUMENT(float, Value);
		CIRCUIT_ARGUMENT(float, Min);
		CIRCUIT_ARGUMENT(float, Max);
		CIRCUIT_END_ARGS();

		CIRCUIT_API void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;
		CIRCUIT_API void OnInputEvent(InputEvent& inputEvent) override;


		CIRCUIT_API float GetValue() const;
		CIRCUIT_API void SetValue(float value);

		CIRCUIT_API float GetMinValue() const;
		CIRCUIT_API void SetMinValue(float value);

		CIRCUIT_API float GetMaxValue() const;
		CIRCUIT_API void SetMaxValue(float value);

		CIRCUIT_API float GetValueNormalized();
	private:
		float m_Value;
		//unsigned int m_OnValueChangeHandle;
		float m_MinValue;
		float m_MaxValue;

		static constexpr uint32_t s_sliderWidth = 200;
		static constexpr uint32_t s_sliderHeight = 10;
		static constexpr uint32_t s_sliderHandleRadius = 10;
	};
}
