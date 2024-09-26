#pragma once
#ifdef INPUTMODULE_DLL_EXPORT
#define INPUTMODULE_API __declspec(dllexport)
#else
#define INPUTMODULE_API __declspec(dllimport)
#endif
