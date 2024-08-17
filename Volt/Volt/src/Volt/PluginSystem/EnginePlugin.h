#pragma once

#include "Volt/PluginSystem/Plugin.h"

namespace Volt
{
	class EnginePlugin : public Plugin
	{
	public:
		~EnginePlugin() override = default;
	};
}

#define VT_REGISTER_ENGINE_PLUGIN(pluginTypename) \
	VT_REGISTER_PLUGIN(pluginTypename, Volt::Plugin::Type::Engine)
