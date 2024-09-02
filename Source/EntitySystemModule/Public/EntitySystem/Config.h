#pragma once

#ifdef ENTITYSYSTEMMODULE_DLL_EXPORT
#define VTES_API __declspec(dllexport)
#else
#define VTES_API __declspec(dllimport)
#endif
