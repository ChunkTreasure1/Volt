#pragma once
#ifdef INPUTMODULE_BUILD_DLL
#define INPUTMODULE_API __declspec(dllexport)
#else
#define INPUTMODULE_API __declspec(dllimport)
#endif
