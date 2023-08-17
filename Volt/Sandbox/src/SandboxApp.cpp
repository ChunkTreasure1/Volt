#include "sbpch.h"

#include "Sandbox/Sandbox.h"

#include <Volt/Core/Application.h>
#include <Volt/EntryPoint.h>

#include <Volt/Platform/ExceptionHandling.h>

class SandboxApp : public Volt::Application
{
public:
	SandboxApp(const Volt::ApplicationInfo& appInfo)
		: Volt::Application(appInfo)
	{
		Sandbox* sandbox = new Sandbox();
		PushLayer(sandbox);
	}
};

inline void Create(SandboxApp*& appPtr, const Volt::ApplicationInfo& info)
{
	appPtr = new SandboxApp(info);
}

Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath)
{
	Volt::ApplicationInfo info{};
	info.iconPath = "Editor/Textures/Icons/icon_volt.dds";
	info.projectPath = appPath;
	info.useVSync = false;
	info.enableSteam = true;
	info.enableImGui = true;
	SandboxApp* app;
	Create(app, info);

	return app;
}
