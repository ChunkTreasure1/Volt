#include "circuitpch.h"
#include "SliderWidget.h"
#include "Circuit/WidgetBuilder.h"
#include "Circuit/CircuitColor.h"
#include "Circuit/Widgets/Primitives/RectWidget.h"
#include "Circuit/Widgets/Primitives/CircleWidget.h"
#include "Delegates/Observer.h"



SliderWidget::SliderWidget()
{
}


SliderWidget::~SliderWidget()
{
	m_Value->GetOnChangeDelegate().Remove(m_OnValueChangeHandle);

}

void SliderWidget::Build(const Arguments& args)
{
	m_MinValue = args._Min;
	m_MaxValue = args._Max;
	m_Value = args._Value;


	const uint32_t sliderWidth = 100;
	const uint32_t sliderHeight = 20;
	const float endOfFirstRect = sliderWidth * GetValueNormalized();

	std::shared_ptr<RectWidget> LeftRect = AddChildWidget<RectWidget>(CreateWidget(RectWidget)
		.Width(endOfFirstRect)
		.Height(sliderHeight)
		.Color(CircuitColor(CircuitColor(0, 255, 0, 255)))
		.X(0)
		.Y(0));

	std::shared_ptr<RectWidget> RightRect = AddChildWidget<RectWidget>(CreateWidget(RectWidget)
		.Width(sliderWidth - endOfFirstRect)
		.Height(sliderHeight)
		.Color(CircuitColor(CircuitColor(255, 0, 0, 255)))
		.X(endOfFirstRect)
		.Y(0));

	std::shared_ptr<CircleWidget> BigCircle = AddChildWidget<CircleWidget>(CreateWidget(CircleWidget)
		.Radius((sliderHeight / 2) * 1.1f)
		.Color(CircuitColor(255, 255, 255, 255))
		.X(endOfFirstRect)
		.Y(sliderHeight / 2));

	std::shared_ptr<CircleWidget> SmallCircle = AddChildWidget<CircleWidget>(CreateWidget(CircleWidget)
		.Radius((sliderHeight / 2) * 0.7f)
		.Color(CircuitColor(0, 255, 0, 255))
		.X(endOfFirstRect)
		.Y(sliderHeight / 2));

	m_OnValueChangeHandle = m_Value->GetOnChangeDelegate().Add([this, LeftRect, RightRect, BigCircle, SmallCircle](float value)
	{
		const uint32_t sliderWidth = 100;
		//const uint32_t sliderHeight = 20;
		const float endOfFirstRect = sliderWidth * GetValueNormalized();

		LeftRect->SetWidth(endOfFirstRect);
		RightRect->SetWidth(sliderWidth - endOfFirstRect);
		RightRect->SetX(endOfFirstRect);
		BigCircle->SetX(endOfFirstRect);
		SmallCircle->SetX(endOfFirstRect);
	});
}

float SliderWidget::GetValue() const
{
	return m_Value->GetValue();
}

void SliderWidget::SetValue(float value)
{
	(*m_Value) = value;
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
	return (m_Value->Get() - m_MinValue) / (m_MaxValue - m_MinValue);
}
