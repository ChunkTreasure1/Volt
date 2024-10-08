#pragma once

#ifdef ASSETSYSTEMMODULE_DLL_EXPORT
#define VTAS_API __declspec(dllexport)
#else
#define VTAS_API __declspec(dllimport)
#endif
