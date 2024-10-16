#pragma once
#ifdef SUBSYSTEMMODULE_DLL_EXPORT
#define SUBSYSTEMMODULE_API __declspec(dllexport)
#else
#define SUBSYSTEMMODULE_API __declspec(dllimport)
#endif
