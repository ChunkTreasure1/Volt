#pragma once

#include "Volt/PluginSystem/PluginDefines.h"

#include <string_view>

#define VT_PLUGIN_SYSTEM_VERSION 1u

namespace Volt
{
	class Plugin
	{
	public:
		enum class Error
		{
			None,
			InvalidSystemVersion
		};

		enum class Type
		{
			Engine,
			Game
		};

		virtual ~Plugin() = default;
		virtual uint32_t GetVersion() const = 0;
		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetDescription() const = 0;
		virtual std::string_view GetCategory() const = 0;

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}

struct PluginInitializationInfo
{
	uint32_t pluginSystemVersion;
	Volt::Plugin::Error outError;
	Volt::Plugin::Type outType;
};

extern "C"
{
	VT_EXPORT_DLL Volt::Plugin* CreatePluginInstance(PluginInitializationInfo& initializationInfo);
	VT_EXPORT_DLL void DestroyPluginInstance(Volt::Plugin* plugin);
}

#define VT_REGISTER_PLUGIN(pluginTypename, pluginType)											\
VT_EXPORT_DLL Volt::Plugin* CreatePluginInstance(PluginInitializationInfo& initializationInfo)	\
{																								\
	if (initializationInfo.pluginSystemVersion != VT_PLUGIN_SYSTEM_VERSION)						\
	{																							\
		initializationInfo.outError = Volt::Plugin::Error::InvalidSystemVersion;				\
		return nullptr;																			\
	}																							\
																								\
	initializationInfo.outError = Volt::Plugin::Error::None;									\
	initializationInfo.outType = pluginType;													\
																								\
	pluginTypename* pluginInstance = new pluginTypename();										\
	return pluginInstance;																		\
}																								\
																								\
VT_EXPORT_DLL void DestroyPluginInstance(Volt::Plugin* plugin) 									\
{																								\
	delete plugin;																				\
}
