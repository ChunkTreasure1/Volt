#pragma once

#ifdef JOBSYSTEMMODULE_DLL_EXPORT
#define VTJS_API __declspec(dllexport)
#else
#define VTJS_API __declspec(dllimport)
#endif
