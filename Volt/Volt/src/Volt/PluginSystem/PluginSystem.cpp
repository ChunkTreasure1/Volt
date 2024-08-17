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
		VT_LOGC(Info, LogPuginSystem, "Started loading of plugins!");
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
						VT_LOGC(Warning, LogPuginSystem, "The dependency {} of plugin {} was not found! Plugin might not work as expected!", pluginDependency, pluginDef.name);
					}
				}
			}
		}
		VT_LOGC(Info, LogPuginSystem, "Finished loading of plugins!");
    }

	void PluginSystem::UnloadPlugins()
	{
		for (const auto& plugin : m_loadedGamePlugins)
		{
			m_pluginFactories.at(plugin.binaryFilepath)->DestroyInstance(plugin.pluginPtr);
			m_pluginFactories.erase(plugin.binaryFilepath);

			DynamicLibraryManager::Get().UnloadDynamicLibrary(plugin.binaryFilepath);
		}

		for (const auto& plugin : m_loadedEnginePlugins)
		{
			m_pluginFactories.at(plugin.binaryFilepath)->DestroyInstance(plugin.pluginPtr);
			m_pluginFactories.erase(plugin.binaryFilepath);

			DynamicLibraryManager::Get().UnloadDynamicLibrary(plugin.binaryFilepath);
		}

		m_loadedGamePlugins.clear();
		m_loadedEnginePlugins.clear();
		m_pluginFactories.clear();
	}

	void PluginSystem::InitializePlugins()
	{
		const auto& dependencyGraph = m_pluginRegistry.GetPluginDependencyGraph();

		Vector<UUID64> initialEnginePlugins;
		Vector<UUID64> initialGamePlugins;
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

			const auto nodePluginType = m_guidToIndexMap.at(node.nodeData).pluginType;
			if (nodePluginType == Plugin::Type::Engine)
			{
				initialEnginePlugins.emplace_back(node.id);
			}
			else if (nodePluginType == Plugin::Type::Game)
			{
				initialGamePlugins.emplace_back(node.id);
			}
		}

		// We first want to initialize engine plugins and after that, game plugins.
		// This is because game plugins may depend on engine plugins.

		for (const auto& nodeId : initialEnginePlugins)
		{
			InitializePluginAndDependencies(nodeId, dependencyGraph);
		}

		for (const auto& nodeId : initialGamePlugins)
		{
			InitializePluginAndDependencies(nodeId, dependencyGraph);
		}
	}

	void PluginSystem::ShutdownPlugins()
	{
		for (const auto& plugin : m_loadedGamePlugins)
		{
			plugin.pluginPtr->Shutdown();
		}

		for (const auto& plugin : m_loadedEnginePlugins)
		{
			plugin.pluginPtr->Shutdown();
		}
	}

	bool PluginSystem::LoadPlugin(const PluginDefinition& pluginDefinition)
	{
		const auto& binaryFilepath = pluginDefinition.binaryFilepath;

		DLLHandle libHandle = DynamicLibraryManager::Get().LoadDynamicLibrary(binaryFilepath);
		if (libHandle == nullptr)
		{
			return false;
		}

		PFN_PluginCreateInstance createFunc = reinterpret_cast<PFN_PluginCreateInstance>(VT_GET_PROC_ADDRESS(libHandle, PLUGIN_CREATE_FUNC_NAME));
		PFN_PluginDestroyInstance destroyFunc = reinterpret_cast<PFN_PluginDestroyInstance>(VT_GET_PROC_ADDRESS(libHandle, PLUGIN_DESTROY_FUNC_NAME));

		Scope<PluginFactory> pluginFactory = CreateScope<PluginFactory>(createFunc, destroyFunc);

		PluginInitializationInfo initializationInfo{};
		initializationInfo.pluginSystemVersion = VT_PLUGIN_SYSTEM_VERSION;
		initializationInfo.outError = Plugin::Error::None;

		Plugin* pluginPtr = pluginFactory->CreateInstance(initializationInfo);
		
		if (pluginPtr == nullptr || initializationInfo.outError != Plugin::Error::None)
		{
			VT_LOGC(Error, LogPuginSystem, "Unable to create instance of plugin {}!", binaryFilepath.string());
			DynamicLibraryManager::Get().UnloadDynamicLibrary(binaryFilepath);
			return false;
		}

		if (initializationInfo.outType == Plugin::Type::Game)
		{
			m_loadedGamePlugins.emplace_back(reinterpret_cast<GamePlugin*>(pluginPtr), binaryFilepath);
			m_guidToIndexMap[pluginDefinition.guid] = { m_loadedGamePlugins.size() - 1, initializationInfo.outType };
		}
		else if (initializationInfo.outType == Plugin::Type::Engine)
		{
			m_loadedEnginePlugins.emplace_back(reinterpret_cast<EnginePlugin*>(pluginPtr), binaryFilepath);
			m_guidToIndexMap[pluginDefinition.guid] = { m_loadedEnginePlugins.size() - 1, initializationInfo.outType };
		}
		else
		{
			VT_LOGC(Error, LogPuginSystem, "Invalid plugin type found in plugin {}!", binaryFilepath.string());
			DynamicLibraryManager::Get().UnloadDynamicLibrary(binaryFilepath);
			return false;
		}

		m_pluginFactories[binaryFilepath] = std::move(pluginFactory);
		VT_LOGC(Info, LogPuginSystem, "Plugin {} has been loaded and created!", pluginDefinition.name);
		return true;
	}

	void PluginSystem::InitializePluginAndDependencies(UUID64 nodeId, const Graph<VoltGUID, uint32_t>& dependencyGraph)
	{
		const auto& node = dependencyGraph.GetNodeFromID(nodeId);
		if (!m_guidToIndexMap.contains(node.nodeData))
		{
			return;
		}

		const IndexData& pluginIndexData = m_guidToIndexMap.at(node.nodeData);

		if (pluginIndexData.pluginType == Plugin::Type::Engine)
		{
			auto& pluginContainer = m_loadedEnginePlugins.at(pluginIndexData.index);
			if (!pluginContainer.initialized)
			{
				pluginContainer.pluginPtr->Initialize();
				pluginContainer.initialized = true;
			}
		}
		else if (pluginIndexData.pluginType == Plugin::Type::Game)
		{
			auto& pluginContainer = m_loadedGamePlugins.at(pluginIndexData.index);
			if (!pluginContainer.initialized)
			{
				pluginContainer.pluginPtr->Initialize();
				pluginContainer.initialized = true;
			}
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
