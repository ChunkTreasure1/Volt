#pragma once
#include "Circuit/Window/WindowInterfaceDefines.h"
#include "Circuit/CircuitDrawCommand.h"

#include "Circuit/Widgets/Widget.h"

#include <vector>

namespace Circuit
{
	class Widget;

	class CircuitWindow
	{
	public:
		CIRCUIT_API CircuitWindow(InterfaceWindowHandle windowHandle);
		CIRCUIT_API ~CircuitWindow() = default;

		CIRCUIT_API InterfaceWindowHandle GetInterfaceWindowHandle() const;

		CIRCUIT_API const glm::u16vec2& GetWindowSize() const;
		CIRCUIT_API void Resize(const glm::vec2& size);

		CIRCUIT_API std::vector<CircuitDrawCommand> GetDrawCommands();

	private:
		const InterfaceWindowHandle m_interfaceWindowHandle;

		glm::u16vec2 m_windowSize;

		std::unique_ptr<Widget> m_Widget;

	};
}
