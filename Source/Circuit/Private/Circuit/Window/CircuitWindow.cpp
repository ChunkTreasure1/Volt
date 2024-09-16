#include "circuitpch.h"
#include "Window/CircuitWindow.h"
#include "Circuit/CircuitPainter.h"

#include "Circuit/Rendering/CircuitRenderer.h"

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

namespace Circuit
{
	CircuitWindow::CircuitWindow(Volt::WindowHandle windowHandle)
		: m_windowHandle(windowHandle)
	{
		m_renderer = CreateRef<CircuitRenderer>(*this);
	}

	Volt::WindowHandle CircuitWindow::GetWindowHandle() const
	{
		return m_windowHandle;
	}
	
	glm::u16vec2 CircuitWindow::GetWindowSize() const
	{
		Volt::Window& window = Volt::WindowManager::Get().GetWindow(m_windowHandle);
		return { window.GetWidth(), window.GetHeight() };
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

	void CircuitWindow::SetWidget(std::unique_ptr<Widget> widget)
	{
		m_widget = std::move(widget);
	}

	void CircuitWindow::OnRender()
	{
		m_renderer->OnRender();
	}
}
