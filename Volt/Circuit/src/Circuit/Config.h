#pragma once

#ifdef CIRCUIT_BUILD_DLL
#define CIRCUIT_API __declspec(dllexport)
#else
#define CIRCUIT_API __declspec(dllimport)
#endif
