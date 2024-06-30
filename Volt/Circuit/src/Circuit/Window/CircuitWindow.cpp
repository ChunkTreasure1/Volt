#include "circuitpch.h"
#include "CircuitWindow.h"
namespace Circuit
{
	CircuitWindow::CircuitWindow(InterfaceWindowHandle windowHandle)
		: m_interfaceWindowHandle(windowHandle)
	{
	}
	InterfaceWindowHandle CircuitWindow::GetInterfaceWindowHandle() const
	{
		return m_interfaceWindowHandle;
	}
	const glm::u16vec2& CircuitWindow::GetWindowSize() const
	{
		return m_windowSize;
	}
	void CircuitWindow::Resize(const glm::vec2& size)
	{
		m_windowSize = size;
	}

	std::vector<CircuitDrawCommand> CircuitWindow::GetDrawCommands()
	{
		return std::vector<CircuitDrawCommand>();
	}
}
