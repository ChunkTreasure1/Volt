#pragma once

#include "Volt-Core/Config.h"

#include <SubSystem/SubSystem.h>

#include <CoreUtilities/Containers/Map.h>

namespace Volt
{
	typedef void* DLLHandle;

	class VTCORE_API DynamicLibraryManager : public SubSystem
	{
	public:
		DynamicLibraryManager();
		~DynamicLibraryManager();

		VT_NODISCARD DLLHandle LoadDynamicLibrary(const std::filesystem::path& binaryFilepath, bool& outExternallyLoaded);
		bool UnloadDynamicLibrary(const std::filesystem::path& binaryFilepath);

		VT_NODISCARD VT_INLINE static DynamicLibraryManager& Get() { return *s_instance; }

		VT_DECLARE_SUBSYSTEM("{FCB778C9-2E7F-4E47-AFC7-A6186AC89ACE}"_guid)

	private:
		inline static DynamicLibraryManager* s_instance = nullptr;

		vt::map<std::filesystem::path, DLLHandle> m_loadedDynamicLibraries;
	};
}
