#include "Launcher/GameLayer.h"

#include "Testing/RenderingTestingLayer.h"

#include <Volt/EntryPoint.h>
#include <Volt/Core/Application.h>

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>
#include <istream>

class LauncherApp : public Volt::Application
{
public:
	LauncherApp(const Volt::ApplicationInfo& appInfo)
		: Volt::Application(appInfo)
	{
		//GameLayer* testing = new GameLayer();
		//PushLayer(testing);
	
		RenderingTestingLayer* testingLayer = new RenderingTestingLayer();
		PushLayer(testingLayer);
	}
private:
};

Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath)
{
	Volt::ApplicationInfo info{};
	info.iconPath = "Editor/Textures/Icons/icon_volt.dds";
	info.projectPath = appPath;
	info.useVSync = false;
	info.enableSteam = false;
	info.enableImGui = false;
	info.isRuntime = true;
	info.width = 1600;
	info.height = 900;

	return new LauncherApp(info);
}
