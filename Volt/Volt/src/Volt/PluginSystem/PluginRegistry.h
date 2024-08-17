#pragma once

#include "Volt/PluginSystem/PluginDefinition.h"

#include <LogModule/LogCategory.h>

#include <CoreUtilities/VoltGUID.h>
#include <CoreUtilities/Containers/Map.h>
#include <CoreUtilities/Containers/Graph.h>

VT_DECLARE_LOG_CATEGORY(LogPuginSystem, LogVerbosity::Trace);

namespace Volt
{
	class PluginRegistry
	{
	public:
		PluginRegistry();
		~PluginRegistry();

		void FindAndRegisterPluginsInDirectory(const std::filesystem::path& directory);
		VT_NODISCARD const PluginDefinition& GetPluginDefinitionByName(const std::string& name) const;

		void BuildPluginDependencies();
		VT_NODISCARD VT_INLINE const Graph<VoltGUID, uint32_t>& GetPluginDependencyGraph() const { return m_pluginDependencyGraph; }

	private:
		void DeserializePlugin(const std::filesystem::path& filepath);

		vt::map<VoltGUID, PluginDefinition> m_registeredPlugins;

		Graph<VoltGUID, uint32_t> m_pluginDependencyGraph;
		vt::map<VoltGUID, UUID64> m_guidToNodeId;
	};
}
