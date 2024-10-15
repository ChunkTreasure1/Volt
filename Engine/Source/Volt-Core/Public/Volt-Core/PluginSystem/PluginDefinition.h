#pragma once

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/VoltGUID.h>

namespace Volt
{
	struct PluginDefinition
	{
		std::string name;
		std::filesystem::path filepath; // Filepath to the plugin description file
		std::filesystem::path binaryFilepath; // Filepath to the plugin binary file
		VoltGUID guid;

		Vector<std::string> pluginDependencies;
	
		VT_INLINE bool IsValid() const
		{
			return guid != VoltGUID::Null();
		}
	};
}
