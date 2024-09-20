#pragma once
#include "Circuit/Widgets/Widget.h"

#include <Volt-Assets/Assets/Font.h>

namespace Volt
{
	class Font;
}

namespace Circuit
{
	class CIRCUIT_API TextWidget : public Widget
	{
	public:
		TextWidget();
		virtual ~TextWidget();

		CIRCUIT_BEGIN_ARGS(TextWidget)
		{
		};
		CIRCUIT_ARGUMENT(std::string, Text);
		CIRCUIT_END_ARGS();

		void Build(const Arguments& args);

		virtual void OnPaint(CircuitPainter& painter) override;
	private:
		std::string m_text;
	
		Ref<Volt::Font> m_font;
	};
}
