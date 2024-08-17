#include "vtpch.h"
#include "DynamicLibraryManager.h"

#include <CoreUtilities/DynamicLibraryHelpers.h>

namespace Volt
{
	DynamicLibraryManager::DynamicLibraryManager()
	{
		VT_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	DynamicLibraryManager::~DynamicLibraryManager()
	{
		s_instance = nullptr;
	}

	VT_NODISCARD DLLHandle DynamicLibraryManager::LoadDynamicLibrary(const std::filesystem::path& binaryFilepath)
	{
		DLLHandle handle = VT_LOAD_LIBRARY(binaryFilepath.string().c_str());

		if (!handle)
		{
			VT_LOGC(Error, LogApplication, "Unable to load DLL {}!", binaryFilepath.string());
			return nullptr;
		}

		m_loadedDynamicLibraries[binaryFilepath] = handle;
		return handle;
	}

	bool DynamicLibraryManager::UnloadDynamicLibrary(const std::filesystem::path& binaryFilepath)
	{
		if (!m_loadedDynamicLibraries.contains(binaryFilepath))
		{
			return false;
		}

		VT_FREE_LIBRARY(m_loadedDynamicLibraries.at(binaryFilepath));
		m_loadedDynamicLibraries.erase(binaryFilepath);
	
		return true;
	}
}
