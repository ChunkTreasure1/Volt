#pragma once

#ifdef WINDOWMODULE_BUILD_DLL
#define WINDOWMODULE_API __declspec(dllexport)
#else
#define WINDOWMODULE_API __declspec(dllimport)
#endif
