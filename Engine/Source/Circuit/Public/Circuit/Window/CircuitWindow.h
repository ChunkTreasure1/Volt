#pragma once

#include "Circuit/CircuitDrawCommand.h"

#include "Circuit/Widgets/Widget.h"

#include <WindowModule/WindowHandle.h>

#include <CoreUtilities/Core.h>

#include <vector>

namespace Circuit
{
	class CircuitRenderer;
	class Widget;

	class CircuitWindow
	{
	public:
		CIRCUIT_API CircuitWindow(Volt::WindowHandle windowHandle);
		CIRCUIT_API ~CircuitWindow() = default;

		CIRCUIT_API Volt::WindowHandle GetWindowHandle() const;

		CIRCUIT_API glm::u16vec2 GetWindowSize() const;
		CIRCUIT_API void Resize(const glm::vec2& size);

		CIRCUIT_API std::vector<CircuitDrawCommand> GetDrawCommands();

		//takes ownership of the widget
		CIRCUIT_API void SetWidget(Ref<Widget> widget);


		void OnRender();
	private:
		const Volt::WindowHandle m_windowHandle;

		Ref<CircuitRenderer> m_renderer;

		glm::u16vec2 m_windowSize;
		std::string m_title;

		Ref<Widget> m_widget;

	};
}
