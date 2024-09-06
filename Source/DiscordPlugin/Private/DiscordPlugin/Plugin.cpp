#include "Plugin.h"

#include "DiscordPlugin/DiscordManager.h"

#include <Volt/Events/ApplicationEvents.h>

#include <LogModule/Log.h>

DiscordPlugin::DiscordPlugin()
{

}

void DiscordPlugin::Initialize()
{
	VT_ENSURE(s_instance == nullptr);
	s_instance = this;
	m_discordManager = CreateScope<DiscordManager>();
}

void DiscordPlugin::Shutdown()
{
	m_discordManager = nullptr;
	s_instance = nullptr;
}

//void DiscordPlugin::OnEvent(Volt::Event& event)
//{
//	Volt::EventDispatcher dispatcher{ event };
//
//	dispatcher.Dispatch<Volt::AppUpdateEvent>([&](Volt::AppUpdateEvent e) 
//	{
//		m_discordManager->Update();
//
//		return false;
//	});
//}
