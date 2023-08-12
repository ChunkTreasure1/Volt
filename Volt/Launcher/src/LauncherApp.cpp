#include "Launcher/GameLayer.h"
#include "Launcher/TestingLayer.h"

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
		TestingLayer* testing = new TestingLayer();
		PushLayer(testing);
	}

	static void LoadWindowSettings(Volt::ApplicationInfo& info);
private:
};

Volt::Application* Volt::CreateApplication(const std::filesystem::path& appPath)
{
	Volt::ApplicationInfo info{};
	info.enableImGui = false;
	info.width = 1600;
	info.height = 900;
	info.windowMode = WindowMode::Windowed;
	info.title = "Vipertrace";
	info.cursorPath = "Assets/UI/Sprites/cursor.dds";
	info.iconPath = "Assets/UI/Sprites/GUI/GUI_pebbles.dds";
	info.isRuntime = true;
	info.projectPath = appPath;
	info.enableSteam = true;

	LauncherApp::LoadWindowSettings(info);
	return new LauncherApp(info);
}

void LauncherApp::LoadWindowSettings(Volt::ApplicationInfo& info)
{
	std::filesystem::path path = info.projectPath.parent_path() / "Assets/Settings/GameSettings.yaml";
	std::ifstream file(path);
	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());

	if (root["WindowMode"])
	{
		Volt::WindowMode windowMode = (Volt::WindowMode)root["WindowMode"].as<uint32_t>();
		switch (windowMode)
		{
			case Volt::WindowMode::Windowed:
				info.windowMode = Volt::WindowMode::Windowed;
				break;
			case Volt::WindowMode::Fullscreen:
				info.windowMode = Volt::WindowMode::Fullscreen;
				break;
			case Volt::WindowMode::Borderless:
				info.windowMode = Volt::WindowMode::Borderless;
				break;
			default:
				info.windowMode = Volt::WindowMode::Borderless;
				break;
		}
	}
	if (root["Vsync"])
	{
		info.useVSync = root["Vsync"].as<bool>();
	}
	else
	{
		info.useVSync = true;
	}

	file.close();
}
