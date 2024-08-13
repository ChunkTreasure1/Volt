#pragma once

#ifdef VTJS_BUILD_DLL
#define VTJS_API __declspec(dllexport)
#else
#define VTJS_API __declspec(dllimport)
#endif
