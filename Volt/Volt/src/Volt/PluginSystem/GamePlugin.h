#pragma once

#include "Volt/PluginSystem/Plugin.h"

namespace Volt
{
	class GamePlugin : public Plugin
	{
	public:
	};
}

#define VT_REGISTER_GAME_PLUGIN(pluginTypename) \
	VT_REGISTER_PLUGIN(pluginTypename, Volt::Plugin::Type::Game)
