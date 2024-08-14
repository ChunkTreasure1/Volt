#pragma once
#ifdef EVENTMODULE_BUILD_DLL
#define EVENTMODULE_API __declspec(dllexport)
#else
#define EVENTMODULE_API __declspec(dllimport)
#endif
