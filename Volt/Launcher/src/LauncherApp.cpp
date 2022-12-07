#include "Launcher/GameLayer.h"
#include "Launcher/FinalLayer.h"

#include <Volt/EntryPoint.h>
#include <Volt/Core/Application.h>

class LauncherApp : public Volt::Application
{
public:
	LauncherApp(const Volt::ApplicationInfo& appInfo)
		: Volt::Application(appInfo)
	{
		GameLayer* game = new GameLayer();
		PushLayer(game);

		PushLayer(new FinalLayer(game->GetSceneRenderer()));

		game->LoadStartScene();
	}

private:
};

Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath)
{
	Volt::ApplicationInfo info{};
	info.enableImGui = false;
	info.width = 1920;
	info.height = 1080;
	info.title = "Spite - The Yellow Plague";
	info.cursorPath = "Assets/UI/Assets/cursor.dds";
	info.iconPath = "Assets/UI/Assets/YellowKingIcon.dds";
	info.useVSync = true;
	info.windowMode = WindowMode::Borderless;
	info.isRuntime = true;

	return new LauncherApp(info);
}