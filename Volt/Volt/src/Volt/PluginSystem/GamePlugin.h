#pragma once

#include "Volt/PluginSystem/Plugin.h"

#include <EventModule/Event.h>

namespace Volt
{
	class GamePlugin : public Plugin
	{
	public:
		virtual void OnEvent(Event& event) {}
	};
}

#define VT_REGISTER_GAME_PLUGIN(pluginTypename) \
	VT_REGISTER_PLUGIN(pluginTypename, Volt::Plugin::Type::Game)
