#pragma once

#include "Volt/Core/Application.h"

#include <filesystem>

extern Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath);

namespace Volt
{
	int Main(const std::filesystem::path& appPath)
	{
		Application* app = Volt::CreateApplication(appPath);
		app->Run();

		delete app;
		return 0;
	}
}

#ifdef VT_DIST

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE aHInstance, HINSTANCE aPrevHInstance, PSTR aCmdLine, int aCmdShow)
{
	return Volt::Main(aCmdLine);
}

#else

int main(int argc, char** argv)
{
	return Volt::Main(argv[0]);
}

#endif