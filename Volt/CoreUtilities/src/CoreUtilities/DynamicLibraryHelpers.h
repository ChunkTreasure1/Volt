#pragma once

#include "CoreUtilities/Config.h"
#include "CoreUtilities/CompilerTraits.h"

#ifdef VT_PLATFORM_WINDOWS

#include "CoreUtilities/Platform/Windows/VoltWindows.h"

#define VT_LOAD_LIBRARY(name) ::LoadLibraryA(name)
#define VT_GET_CURRENT_MODULE() ::GetModuleHandle(nullptr)
#define VT_GET_MODULE_FILENAME(module, filename, size) ::GetModuleFileName(module, filename, size)
#define VT_SHARED_LIBRARY_SUPPORTED true
#define VT_SHARED_LIBRARY_EXTENSION ".dll"
#define VT_GET_PROC_ADDRESS(libHandle, name) ::GetProcAddress((HMODULE)(libHandle), name)
#define VT_FREE_LIBRARY(libHandle) ::FreeLibrary((HMODULE)(libHandle))

#else

#define VT_LOAD_LIBRARY(name)
#define VT_GET_CURRENT_MODULE() nullptr
#define VT_GET_MODULE_FILENAME(module, filename, size) ""
#define VT_SHARED_LIBRARY_SUPPORTED false
#define VT_SHARED_LIBRARY_EXTENSION ""
#define VT_GET_PROC_ADDRESS(libHandle, name) nullptr
#define VT_FREE_LIBRARY(libHandle)
typedef void* HMODULE;

#endif

#include <string_view>

class DynamicLibraryHelper
{
public:
	VTCOREUTIL_API DynamicLibraryHelper(std::string_view libraryFilepath);
	VTCOREUTIL_API DynamicLibraryHelper(const DynamicLibraryHelper& other) = delete;
	VTCOREUTIL_API ~DynamicLibraryHelper();

	VTCOREUTIL_API void Free();
	VTCOREUTIL_API void Release();
	VTCOREUTIL_API void Load(std::string_view libraryFilepath);

	VT_INLINE bool IsLoaded() const { return m_moduleHandle != nullptr; }

	template<typename ProcFunc>
	VT_INLINE ProcFunc GetProcAddress(std::string_view procName) const
	{
		return static_cast<ProcFunc>(VT_GET_PROC_ADDRESS(m_moduleHandle, procName.data()));
	}

private:
	HMODULE m_moduleHandle;
};
