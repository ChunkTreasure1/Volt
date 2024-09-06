#include "circuitpch.h"
#include "Window/CircuitWindow.h"
#include "Circuit/CircuitPainter.h"

namespace Circuit
{
	CircuitWindow::CircuitWindow(Volt::WindowHandle windowHandle, const OpenWindowParams& params)
		: m_windowHandle(windowHandle)
	{
		m_title = params.title;
		m_windowSize = { params.startWidth, params.startHeight };
	}

	Volt::WindowHandle CircuitWindow::GetWindowHandle() const
	{
		return m_windowHandle;
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
