#pragma once
#include "Circuit/Widgets/Widget.h"
#include "Circuit/Window/WindowInterfaceDefines.h"

#include <vector>

namespace Circuit
{
	class CircuitWindow
	{
	public:
		CircuitWindow(InterfaceWindowHandle windowHandle);
		~CircuitWindow() = default;

	private:
		const InterfaceWindowHandle m_WindowHandle;
	};
}
