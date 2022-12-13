#include "sbpch.h"

#include "Sandbox/Sandbox.h"

#include <Volt/Core/Application.h>
#include <Volt/EntryPoint.h>

#include <Volt/Platform/ExceptionHandling.h>

#include <dpp/dpp.h>

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

inline void SendDiscordMessage(const std::filesystem::path& dmpPath)
{
	dpp::cluster bot("");
	dpp::webhook wh("https://discord.com/api/webhooks/1044616206520438825/lA7ONWakE8XwFQbSFY-ip9aleuAiMaGF8WiinDq1-eBLOLSqqD0MdhSNe-7KNMovnApL");

	const std::string user = FileSystem::GetCurrentUserName();

	auto msg = dpp::message(std::format("{0} just crashed! <:ivar_point:1044955145139662878>", user));
	bot.execute_webhook_sync(wh, msg);
}

inline void Create(SandboxApp*& appPtr, const Volt::ApplicationInfo& info)
{
	appPtr = new SandboxApp(info);
}

inline void CreateProxy(std::filesystem::path& dmpPath, SandboxApp*& appPtr, const Volt::ApplicationInfo& info)
{
	__try
	{
		Create(appPtr, info);
	}
	__except (Volt::ExceptionFilterFunction(GetExceptionInformation(), dmpPath))
	{
#ifdef VT_RELEASE
		SendDiscordMessage(dmpPath);
#endif
	}
}

Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath)
{
	Volt::ApplicationInfo info{};
	info.iconPath = "Editor/Textures/Icons/icon_volt.dds";
	info.projectPath = appPath;
	info.useVSync = false;
	
	std::filesystem::path dmpPath;

	SandboxApp* app;
	CreateProxy(dmpPath, app, info);

	return app;
}