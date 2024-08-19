#include "vtpch.h"
#include "PluginSystem.h"

#include "Volt/Project/Project.h"
#include "Volt/PluginSystem/PluginRegistry.h"
#include "Volt/Core/DynamicLibraryManager.h"

#include <CoreUtilities/DynamicLibraryHelpers.h>

namespace Volt
{
	PluginSystem::PluginSystem(PluginRegistry& pluginRegistry)
		: m_pluginRegistry(pluginRegistry)
	{
	}

	void PluginSystem::LoadPlugins(const Project& project)
    {
		VT_LOGC(Info, LogPluginSystem, "Started loading of plugins!");
		auto pluginsToLoad = project.pluginDefinitions;

		// Try to load all plugins defined in project file
		// If a plugin was successfully loaded and has dependencies, we will load those as well.

		for (const auto& pluginDef : pluginsToLoad)
		{
			if (LoadPlugin(pluginDef))
			{
				for (const auto& pluginDependency : pluginDef.pluginDependencies)
				{
					const auto& dependencyDefinition = m_pluginRegistry.GetPluginDefinitionByName(pluginDependency);
					if (dependencyDefinition.IsValid())
					{
						pluginsToLoad.emplace_back(dependencyDefinition);
					}
					else
					{
						VT_LOGC(Warning, LogPluginSystem, "The dependency {} of plugin {} was not found! Plugin might not work as expected!", pluginDependency, pluginDef.name);
					}
				}
			}
		}
		VT_LOGC(Info, LogPluginSystem, "Finished loading of plugins!");
    }

	void PluginSystem::UnloadPlugins()
	{
		for (auto& plugin : m_loadedPlugins)
		{
			m_pluginFactories.at(plugin.nativePlugin.GetBinaryFilepath())->DestroyInstance(plugin.pluginPtr);
			m_pluginFactories.erase(plugin.nativePlugin.GetBinaryFilepath());

			plugin.nativePlugin.Unload();
		}

		m_loadedPlugins.clear();
		m_pluginFactories.clear();
	}

	void PluginSystem::InitializePlugins()
	{
		const auto& dependencyGraph = m_pluginRegistry.GetPluginDependencyGraph();

		Vector<UUID64> initialPlugins;
		for (const auto& node : dependencyGraph.GetNodes())
		{
			if (!node.GetInputEdges().empty())
			{
				continue;
			}

			if (!m_guidToIndexMap.contains(node.nodeData))
			{
				continue;
			}

			initialPlugins.emplace_back(node.id);
		}

		for (const auto& nodeId : initialPlugins)
		{
			InitializePluginAndDependencies(nodeId, dependencyGraph);
		}
	}

	void PluginSystem::ShutdownPlugins()
	{
		for (const auto& plugin : m_loadedPlugins)
		{
			plugin.pluginPtr->Shutdown();
		}
	}

	void PluginSystem::SendEventToPlugins(Volt::Event& event)
	{
	}

	bool PluginSystem::LoadPlugin(const PluginDefinition& pluginDefinition)
	{
		const auto& binaryFilepath = pluginDefinition.binaryFilepath;

		bool externallyLoaded;
		DLLHandle libHandle = DynamicLibraryManager::Get().LoadDynamicLibrary(binaryFilepath, externallyLoaded);
		if (libHandle == nullptr)
		{
			return false;
		}

		NativePluginModule nativePlugin{ binaryFilepath, externallyLoaded };

		PFN_PluginCreateInstance createFunc = reinterpret_cast<PFN_PluginCreateInstance>(VT_GET_PROC_ADDRESS(libHandle, PLUGIN_CREATE_FUNC_NAME));
		PFN_PluginDestroyInstance destroyFunc = reinterpret_cast<PFN_PluginDestroyInstance>(VT_GET_PROC_ADDRESS(libHandle, PLUGIN_DESTROY_FUNC_NAME));

		if (!createFunc || !destroyFunc)
		{
			VT_LOGC(Error, LogPluginSystem, "Unable to find required functions in plugin!");
			return false;
		}

		Scope<PluginFactory> pluginFactory = CreateScope<PluginFactory>(createFunc, destroyFunc);

		PluginInitializationInfo initializationInfo{};
		initializationInfo.pluginSystemVersion = VT_PLUGIN_SYSTEM_VERSION;
		initializationInfo.outError = Plugin::Error::None;

		Plugin* pluginPtr = pluginFactory->CreateInstance(initializationInfo);
		
		if (pluginPtr == nullptr || initializationInfo.outError != Plugin::Error::None)
		{
			if (pluginPtr)
			{
				pluginFactory->DestroyInstance(pluginPtr);
			}

			VT_LOGC(Error, LogPluginSystem, "Unable to create instance of plugin {}!", binaryFilepath.string());
			return false;
		}

		m_loadedPlugins.emplace_back(pluginPtr, std::move(nativePlugin));
		m_guidToIndexMap[pluginDefinition.guid] = { m_loadedPlugins.size() - 1};

		m_pluginFactories[binaryFilepath] = std::move(pluginFactory);
		VT_LOGC(Info, LogPluginSystem, "Plugin {} has been loaded and created!", pluginDefinition.name);
		return true;
	}

	void PluginSystem::InitializePluginAndDependencies(UUID64 nodeId, const Graph<VoltGUID, uint32_t>& dependencyGraph)
	{
		const auto& node = dependencyGraph.GetNodeFromID(nodeId);
		if (!m_guidToIndexMap.contains(node.nodeData))
		{
			return;
		}

		auto& pluginContainer = m_loadedPlugins.at(m_guidToIndexMap.at(node.nodeData));
		if (!pluginContainer.initialized)
		{
			pluginContainer.pluginPtr->Initialize();
			pluginContainer.initialized = true;
		}

		for (const auto& edgeId : node.GetOutputEdges())
		{
			const auto& edge = dependencyGraph.GetEdgeFromID(edgeId);
			InitializePluginAndDependencies(edge.endNode, dependencyGraph);
		}
	}

	PluginFactory::PluginFactory(PFN_PluginCreateInstance createFunc, PFN_PluginDestroyInstance destroyFunc)
		: m_createInstanceFunc(createFunc), m_destroyInstanceFunc(destroyFunc)
	{
	}

	Plugin* PluginFactory::CreateInstance(const PluginInitializationInfo& info)
	{
		return m_createInstanceFunc(info);
	}
	
	void PluginFactory::DestroyInstance(Plugin* plugin)
	{
		m_destroyInstanceFunc(plugin);
	}
}
