#include "vtpch.h"
#include "PluginRegistry.h"

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/DynamicLibraryHelpers.h>
#include <CoreUtilities/FileSystem.h>

VT_DEFINE_LOG_CATEGORY(LogPuginSystem);

namespace Volt
{
	constexpr std::string_view PLUGIN_EXTENSION = ".vtconfig";

	PluginRegistry::PluginRegistry()
	{
	}
	PluginRegistry::~PluginRegistry()
	{
	}

	void PluginRegistry::FindAndRegisterPluginsInDirectory(const std::filesystem::path& directory)
	{
		if (!FileSystem::Exists(directory))
		{
			return;
		}

		VT_LOGC(Info, LogPuginSystem, "Starting registering of plugins in directory {}!", directory.string());

		for (const auto& path : std::filesystem::recursive_directory_iterator(directory))
		{
			if (path.is_directory())
			{
				continue;
			}

			const auto filepath = path.path();
			if (filepath.extension().string() != PLUGIN_EXTENSION)
			{
				continue;
			}
			
			DeserializePlugin(filepath);
		}

		VT_LOGC(Info, LogPuginSystem, "Finished registering of plugins!");
	}

	const PluginDefinition& PluginRegistry::GetPluginDefinitionByName(const std::string& name) const
	{
		for (const auto& [guid, definition] : m_registeredPlugins)
		{
			if (definition.name == name)
			{
				return definition;
			}
		}

		static PluginDefinition nullDefinition;
		return nullDefinition;
	}

	void PluginRegistry::BuildPluginDependencies()
	{
		for (const auto& [guid, pluginDefinition] : m_registeredPlugins)
		{
			if (!m_guidToNodeId.contains(guid))
			{
				m_guidToNodeId[guid] = m_pluginDependencyGraph.AddNode(guid);
			}

			for (const auto& depName : pluginDefinition.pluginDependencies)
			{
				const auto& depDefinition = GetPluginDefinitionByName(depName);
				if (!depDefinition.IsValid())
				{
					continue;
				}

				if (!m_guidToNodeId.contains(depDefinition.guid))
				{
					m_guidToNodeId[guid] = m_pluginDependencyGraph.AddNode(depDefinition.guid);
				}

				m_pluginDependencyGraph.LinkNodes(m_guidToNodeId.at(guid), m_guidToNodeId.at(depDefinition.guid));
			}
		}
	}

	void PluginRegistry::DeserializePlugin(const std::filesystem::path& filepath)
	{
		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(filepath))
		{
			VT_LOGC(Warning, LogPuginSystem, "Unable to open file {}!", filepath.string());
			return;
		}

		if (!streamReader.HasKey("Plugin"))
		{
			VT_LOGC(Warning, LogPuginSystem, "Plugin file {} is invalid!", filepath.string());
		}

		PluginDefinition newPlugin{};

		streamReader.EnterScope("Plugin");

		newPlugin.name = streamReader.ReadAtKey("name", std::string(""));
		if (newPlugin.name.empty())
		{
			VT_LOGC(Error, LogPuginSystem, "Plugin {} does not have a name defined!", filepath.string());
			return;
		}

		std::string guidString = streamReader.ReadAtKey("guid", std::string(""));
		if (guidString.empty())
		{
			newPlugin.guid = VoltGUID::Null();
		}
		else
		{
			newPlugin.guid = VoltGUID::FromStringInternal(guidString.c_str());
		}

		if (newPlugin.guid == VoltGUID::Null())
		{
			VT_LOGC(Error, LogPuginSystem, "Plugin {} does not have a GUID defined!", filepath.string());
			return;
		}

		newPlugin.binaryFilepath = filepath.parent_path() / (filepath.stem().string() + VT_SHARED_LIBRARY_EXTENSION);

		streamReader.ForEach("Plugins", [&]() 
		{
			newPlugin.pluginDependencies.emplace_back(streamReader.ReadValue<std::string>());
		});

		streamReader.ExitScope();

		m_registeredPlugins[newPlugin.guid] = newPlugin;
		VT_LOGC(Info, LogPuginSystem, "Plugin {} has been registered!", newPlugin.name);
	}
}
