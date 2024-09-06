#pragma once

#ifdef WINDOWMODULE_DLL_EXPORT
#define WINDOWMODULE_API __declspec(dllexport)
#else
#define WINDOWMODULE_API __declspec(dllimport)
#endif
