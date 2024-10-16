#pragma once

#include "Volt-Core/Config.h"
#include "Volt-Core/Plugin/Plugin.h"
#include "Volt-Core/PluginSystem/NativePluginModule.h"

#include <SubSystem/SubSystem.h>
#include <EventSystem/Event.h>

#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/Containers/Graph.h>
#include <CoreUtilities/VoltGUID.h>

namespace Volt
{
	struct Project;
	struct PluginDefinition;
	class Plugin;
	class PluginRegistry;

	typedef Plugin* (*PFN_PluginCreateInstance)(const PluginInitializationInfo& info);
	typedef void(*PFN_PluginDestroyInstance)(Plugin* plugin);

	inline static constexpr const char* PLUGIN_CREATE_FUNC_NAME = "CreatePluginInstance";
	inline static constexpr const char* PLUGIN_DESTROY_FUNC_NAME = "DestroyPluginInstance";

	template<typename PluginType>
	struct PluginContainer
	{
		PluginType* pluginPtr;
		NativePluginModule nativePlugin;
		bool initialized = false;
	};

	class VTCORE_API PluginFactory
	{
	public:
		PluginFactory(PFN_PluginCreateInstance createFunc, PFN_PluginDestroyInstance destroyFunc);

		Plugin* CreateInstance(const PluginInitializationInfo& info);
		void DestroyInstance(Plugin* plugin);

	private:
		PFN_PluginCreateInstance m_createInstanceFunc;
		PFN_PluginDestroyInstance m_destroyInstanceFunc;
	};

	class VTCORE_API PluginSystem : public SubSystem
	{
	public:
		void SetPluginRegistry(PluginRegistry* pluginRegistry);

		void LoadPlugins(const Project& project);
		void UnloadPlugins();

		void InitializePlugins();
		void ShutdownPlugins();

		void SendEventToPlugins(Volt::Event& event);

		VT_DECLARE_SUBSYSTEM("{AC054603-4CED-42FD-8BC8-370713EB5EE4}"_guid)

	private:
		bool LoadPlugin(const PluginDefinition& pluginDefinition);
		
		void InitializePluginAndDependencies(UUID64 nodeId, const Graph<VoltGUID, uint32_t>& dependencyGraph);

		PluginRegistry* m_pluginRegistry = nullptr;

		vt::map<std::filesystem::path, Ref<PluginFactory>> m_pluginFactories;
		vt::map<VoltGUID, size_t> m_guidToIndexMap;

		Vector<PluginContainer<Plugin>> m_loadedPlugins;
	};
}
