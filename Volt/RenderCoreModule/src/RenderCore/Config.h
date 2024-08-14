#pragma once

#ifdef VTRC_BUILD_DLL
#define VTRC_API __declspec(dllexport)
#else
#define VTRC_API __declspec(dllimport)
#endif

#ifdef VT_DEBUG

#define VT_ENABLE_SHADER_RUNTIME_VALIDATION

#elif VT_RELEASE

#define VT_ENABLE_SHADER_RUNTIME_VALIDATION

#elif VT_DIST

#endif
