#include "circuitpch.h"
#include "Widgets/TextWidget.h"

#include "CircuitPainter.h"

#include <AssetSystem/AssetManager.h>

Circuit::TextWidget::TextWidget()
{
	m_font = CreateRef<Volt::Font>();
	m_font->Initialize("Engine/Fonts/Futura/futura-light.ttf");
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
	painter.AddRect(GetX(), GetY(), 100, 30, CircuitColor(0xffffffff));
	painter.AddText(GetX(), GetY(), "Test!", m_font, 100.f, CircuitColor(100, 100, 50), 45.f);
}
