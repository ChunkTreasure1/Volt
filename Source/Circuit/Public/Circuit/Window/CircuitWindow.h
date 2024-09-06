#pragma once

#include "Circuit/CircuitDrawCommand.h"

#include "Circuit/Widgets/Widget.h"

#include <WindowModule/WindowHandle.h>

#include <vector>

namespace Circuit
{
	struct OpenWindowParams
	{
		std::string title = "Unnamed Circuit Window";
		uint32_t startWidth = 1280;
		uint32_t startHeight = 720;
	};

	class Widget;

	class CircuitWindow
	{
	public:
		CIRCUIT_API CircuitWindow(Volt::WindowHandle windowHandle, const OpenWindowParams& params = {});
		CIRCUIT_API ~CircuitWindow() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const;

		CIRCUIT_API const glm::u16vec2& GetWindowSize() const;
		CIRCUIT_API void Resize(const glm::vec2& size);

		CIRCUIT_API std::vector<CircuitDrawCommand> GetDrawCommands();

		void OnInputEvent(class InputEvent& inputEvent);

		//takes ownership of the widget
		CIRCUIT_API void SetWidget(std::unique_ptr<Widget> widget);
		template<class WidgetType>
		void SetWidget(std::unique_ptr<WidgetType>& widget)
		{
			SetWidget(std::move(widget));
		}

	private:
		const Volt::WindowHandle m_windowHandle;

		glm::u16vec2 m_windowSize;
		std::string m_title;

		std::unique_ptr<Widget> m_widget;

	};
}
