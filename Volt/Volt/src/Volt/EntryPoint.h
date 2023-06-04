#pragma once

#include "Volt/Core/Application.h"
#include "Volt/Platform/ExceptionHandling.h"

#include <filesystem>

extern Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath);

namespace Volt
{
	void Create(const std::filesystem::path& appPath)
	{
		Application* app = Volt::CreateApplication(appPath);
		app->Run();

		delete app;
	}

	void StartCrashHandler()
	{
		FileSystem::StartProcess("CrashHandler.exe");
	}

	void CreateProxy(std::filesystem::path& dmpPath, const std::filesystem::path& path)
	{
#ifdef VT_DIST
		__try
		{
			Create(path);
		}
		__except (ExceptionFilterFunction(GetExceptionInformation(), dmpPath))
		{
			StartCrashHandler();
		}
#else
		Create(path);
#endif
	}

	int Main(const std::filesystem::path& appPath)
	{
		std::filesystem::path dmpPath;
		CreateProxy(dmpPath, appPath);
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
	if (argc > 1)
	{
		return Volt::Main(argv[1]);
	}

	return Volt::Main("");
}

#endif
