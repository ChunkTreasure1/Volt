#pragma once

#ifdef CIRCUIT_DLL_EXPORT
#define CIRCUIT_API __declspec(dllexport)
#else
#define CIRCUIT_API __declspec(dllimport)
#endif
