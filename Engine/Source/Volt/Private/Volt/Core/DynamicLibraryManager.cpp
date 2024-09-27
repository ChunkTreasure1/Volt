#include "vtpch.h"
#include "Volt/Core/DynamicLibraryManager.h"

#include <CoreUtilities/DynamicLibraryHelpers.h>
#include <CoreUtilities/StringUtility.h>

namespace Volt
{
	DynamicLibraryManager::DynamicLibraryManager()
	{
		VT_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	DynamicLibraryManager::~DynamicLibraryManager()
	{
		VT_ENSURE(m_loadedDynamicLibraries.empty());
		s_instance = nullptr;
	}

	VT_NODISCARD DLLHandle DynamicLibraryManager::LoadDynamicLibrary(const std::filesystem::path& binaryFilepath, bool& outExternallyLoaded)
	{
		DLLHandle handle = VT_GET_MODULE_HANDLE(binaryFilepath.string().c_str());
		outExternallyLoaded = handle != nullptr;

		const std::string cleanFilePath = ::Utility::ReplaceCharacter(binaryFilepath.string(), '/', '\\');

		if (!outExternallyLoaded)
		{
			handle = VT_LOAD_LIBRARY(cleanFilePath.c_str());
		}

		if (!handle)
		{
			VT_LOGC(Error, LogApplication, "Unable to load DLL {}!", cleanFilePath);
			return nullptr;
		}

		m_loadedDynamicLibraries[cleanFilePath] = handle;
		return handle;
	}

	bool DynamicLibraryManager::UnloadDynamicLibrary(const std::filesystem::path& binaryFilepath)
	{
		const std::string cleanFilePath = ::Utility::ReplaceCharacter(binaryFilepath.string(), '/', '\\');

		if (!m_loadedDynamicLibraries.contains(cleanFilePath))
		{
			return false;
		}

		VT_FREE_LIBRARY(m_loadedDynamicLibraries.at(cleanFilePath));
		m_loadedDynamicLibraries.erase(cleanFilePath);
	
		return true;
	}
}
