#pragma once

#include "Volt/PluginSystem/PluginDefinition.h"
#include "Volt/Utility/Version.h"

#include <CoreUtilities/VoltGUID.h>

#include <filesystem>

namespace Volt
{
	struct Project
	{
		Version engineVersion;
		std::string name;
		std::string companyName;

		std::filesystem::path filepath; // Filepath to the .vtproj file
		std::filesystem::path rootDirectory; // The directory containing the .vtproj file
		std::filesystem::path assetsDirectory; // The directory containing the project assets, relative to the rootDirectory
		std::filesystem::path audioDirectory; // The directory containing the audio banks, relative to the rootDirectory

		std::filesystem::path cursorFilepath; // Filepath to the cursor that the project should use when built
		std::filesystem::path iconFilepath; // Filepath to the icon that the project should use when built;
		std::filesystem::path startSceneFilepath; // Filepath to the scene that the project should start with when built;

		Vector<PluginDefinition> pluginDefinitions;

		bool isDeprecated = false;
	};
}
