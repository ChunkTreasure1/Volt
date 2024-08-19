#pragma once

#include "Volt/PluginSystem/PluginDefines.h"

#include <CoreUtilities/VoltGUID.h>

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

		virtual ~Plugin() = default;
		virtual uint32_t GetVersion() const = 0;
		virtual std::string_view GetName() const = 0;
		virtual std::string_view GetDescription() const = 0;
		virtual std::string_view GetCategory() const = 0;
		virtual VoltGUID GetGUID() const = 0;

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}

struct PluginInitializationInfo
{
	uint32_t pluginSystemVersion;
	Volt::Plugin::Error outError;
};

extern "C"
{
	VT_EXPORT_DLL Volt::Plugin* CreatePluginInstance(PluginInitializationInfo& initializationInfo);
	VT_EXPORT_DLL void DestroyPluginInstance(Volt::Plugin* plugin);
}

#ifdef VT_PLUGIN_BUILD_DLL
	#define PLUGIN_API VT_EXPORT_DLL
#else
	#define PLUGIN_API VT_IMPORT_DLL
#endif

#define VT_DECLARE_PLUGIN_GUID(guid)				   \
	inline static constexpr VoltGUID pluginGUID = guid; \
	VT_NODISCARD VT_INLINE VoltGUID GetGUID() const { return pluginGUID; } 

#define VT_REGISTER_PLUGIN(pluginTypename)											\
VT_EXPORT_DLL Volt::Plugin* CreatePluginInstance(PluginInitializationInfo& initializationInfo)	\
{																								\
	static_assert(pluginTypename::pluginGUID != VoltGUID::Null());								\
	if (initializationInfo.pluginSystemVersion != VT_PLUGIN_SYSTEM_VERSION)						\
	{																							\
		initializationInfo.outError = Volt::Plugin::Error::InvalidSystemVersion;				\
		return nullptr;																			\
	}																							\
																								\
	initializationInfo.outError = Volt::Plugin::Error::None;									\
																								\
	pluginTypename* pluginInstance = new pluginTypename();										\
	return pluginInstance;																		\
}																								\
																								\
VT_EXPORT_DLL void DestroyPluginInstance(Volt::Plugin* plugin) 									\
{																								\
	delete plugin;																				\
}
