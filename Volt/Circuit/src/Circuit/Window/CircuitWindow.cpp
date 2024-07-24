#include "circuitpch.h"
#include "CircuitWindow.h"
#include "Circuit/CircuitPainter.h"

namespace Circuit
{
	CircuitWindow::CircuitWindow(InterfaceWindowHandle windowHandle, const OpenWindowParams& params)
		: m_interfaceWindowHandle(windowHandle)
	{
		m_title = params.title;
		m_windowSize = { params.startWidth, params.startHeight };
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
		CircuitPainter painter;
		m_widget->OnPaint(painter);
		return painter.GetCommands();
	}
	void CircuitWindow::OnInputEvent(InputEvent& inputEvent)
	{
		m_widget->OnInputEvent(inputEvent);
	}
	void CircuitWindow::SetWidget(std::unique_ptr<Widget> widget)
	{
		m_widget = std::move(widget);
	}
}
