#pragma once

#ifdef VULKANRHIMODULE_DLL_EXPORT
#define VTVK_API __declspec(dllexport)
#else
#define VTVK_API __declspec(dllimport)
#endif
