#pragma once

#ifdef VTVK_BUILD_DLL
#define VTVK_API __declspec(dllexport)
#else
#define VTVK_API __declspec(dllimport)
#endif
