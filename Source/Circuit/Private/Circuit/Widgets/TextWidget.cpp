#include "circuitpch.h"
#include "Widgets/TextWidget.h"

#include "CircuitPainter.h"

Circuit::TextWidget::TextWidget()
{
}

Circuit::TextWidget::~TextWidget()
{
}

void Circuit::TextWidget::Build(const Arguments& args)
{
	m_text = args._Text;
}

void Circuit::TextWidget::OnPaint(CircuitPainter& painter)
{
	painter.AddRect(100, 100, 100, 30, CircuitColor(0xffffffff));
}
