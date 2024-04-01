#include "sbpch.h"

#include "Sandbox/Sandbox.h"
#include "ProjectUpgrade/ProjectUpgradeLayer.h"

#include <Volt/Core/Application.h>
#include <Volt/EntryPoint.h>

#include <Volt/Platform/ExceptionHandling.h>
#include <Volt/Project/ProjectManager.h>

class SandboxApp : public Volt::Application
{
public:
	SandboxApp(const Volt::ApplicationInfo& appInfo)
		: Volt::Application(appInfo)
	{
		if (Volt::ProjectManager::GetProject().isDeprecated)
		{
			ProjectUpgradeLayer* layer = new ProjectUpgradeLayer();
			PushLayer(layer);
		}
		else
		{
			Sandbox* sandbox = new Sandbox();
			PushLayer(sandbox);
		}
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
	info.useVSync = true;
	info.enableSteam = true;

	SandboxApp* app;
	Create(app, info);

	return app;
}
