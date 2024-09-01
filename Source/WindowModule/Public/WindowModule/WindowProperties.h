#pragma once
#include "WindowModule/WindowMode.h"
#include <filesystem>

#include "Config.h"

namespace Volt
{
	struct WINDOWMODULE_API WindowProperties
	{
		WindowProperties(const std::string& title = "VoltWindow", uint32_t width = 1280, uint32_t height = 720, bool vSync = true, WindowMode windowMode = WindowMode::Windowed)
			: Title(title), Width(width), Height(height), VSync(vSync), WindowMode(windowMode), UseTitlebar(false), UseCustomTitlebar(false)
		{
		}

		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool VSync;
		bool UseTitlebar;
		bool UseCustomTitlebar;
		WindowMode WindowMode;
		std::filesystem::path IconPath;
		std::filesystem::path CursorPath;
	};
}
