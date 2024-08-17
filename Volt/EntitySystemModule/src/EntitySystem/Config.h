#pragma once

#ifdef VTES_BUILD_DLL
#define VTES_API __declspec(dllexport)
#else
#define VTES_API __declspec(dllimport)
#endif
