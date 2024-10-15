#pragma once

#include "Volt-Core/Config.h"

#include <CoreUtilities/Containers/Map.h>

namespace Volt
{
	typedef void* DLLHandle;

	class VTCORE_API DynamicLibraryManager
	{
	public:
		DynamicLibraryManager();
		~DynamicLibraryManager();

		VT_NODISCARD DLLHandle LoadDynamicLibrary(const std::filesystem::path& binaryFilepath, bool& outExternallyLoaded);
		bool UnloadDynamicLibrary(const std::filesystem::path& binaryFilepath);

		VT_NODISCARD VT_INLINE static DynamicLibraryManager& Get() { return *s_instance; }

	private:
		inline static DynamicLibraryManager* s_instance = nullptr;

		vt::map<std::filesystem::path, DLLHandle> m_loadedDynamicLibraries;
	};
}
