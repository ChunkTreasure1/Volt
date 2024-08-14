#pragma once

#ifdef VTAS_BUILD_DLL
#define VTAS_API __declspec(dllexport)
#else
#define VTAS_API __declspec(dllimport)
#endif
