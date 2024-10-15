#pragma once

#define VT_VERSION Version::Create(0, 1, 5)

#ifdef VOLT_CORE_DLL_EXPORT
#define VTCORE_API __declspec(dllexport)
#else
#define VTCORE_API __declspec(dllimport)
#endif
