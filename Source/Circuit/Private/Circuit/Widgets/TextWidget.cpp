#include "circuitpch.h"
#include "Widgets/TextWidget.h"

#include "CircuitPainter.h"

#include <AssetSystem/AssetManager.h>

Circuit::TextWidget::TextWidget() : Circuit::Widget()
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
	const glm::vec2 painterPos = painter.GetAllotedArea().GetPosition();
	painter.AddText(painterPos.x, painterPos.y, m_text, m_font, 100.f, CircuitColor(100, 100, 50), 45.f);
}
