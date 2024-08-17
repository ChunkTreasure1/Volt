#pragma once

#include "Volt/PluginSystem/GamePlugin.h"
#include "Volt/PluginSystem/EnginePlugin.h"

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
		std::filesystem::path binaryFilepath;
		bool initialized = false;
	};

	class PluginFactory
	{
	public:
		PluginFactory(PFN_PluginCreateInstance createFunc, PFN_PluginDestroyInstance destroyFunc);

		Plugin* CreateInstance(const PluginInitializationInfo& info);
		void DestroyInstance(Plugin* plugin);

	private:
		PFN_PluginCreateInstance m_createInstanceFunc;
		PFN_PluginDestroyInstance m_destroyInstanceFunc;
	};

	class PluginSystem
	{
	public:
		PluginSystem(PluginRegistry& pluginRegistry);

		void LoadPlugins(const Project& project);
		void UnloadPlugins();

		void InitializePlugins();
		void ShutdownPlugins();

		void SendEventToPlugins(Volt::Event& event);

	private:
		struct IndexData
		{
			size_t index;
			Plugin::Type pluginType;
		};

		bool LoadPlugin(const PluginDefinition& pluginDefinition);
		
		void InitializePluginAndDependencies(UUID64 nodeId, const Graph<VoltGUID, uint32_t>& dependencyGraph);

		PluginRegistry& m_pluginRegistry;

		vt::map<std::filesystem::path, Scope<PluginFactory>> m_pluginFactories;
		vt::map<VoltGUID, IndexData> m_guidToIndexMap;

		Vector<PluginContainer<GamePlugin>> m_loadedGamePlugins;
		Vector<PluginContainer<EnginePlugin>> m_loadedEnginePlugins;
	};
}
