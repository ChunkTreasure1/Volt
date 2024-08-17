#pragma once

#ifdef VT_PLATFORM_WINDOWS

#define VT_EXPORT_DLL __declspec(dllexport)
#define VT_IMPORT_DLL __declspec(dllimport)

#else

#define VT_EXPORT_DLL
#define VT_IMPORT_DLL

#endif
