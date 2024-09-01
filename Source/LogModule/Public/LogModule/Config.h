#pragma once

#ifdef LOGMODULE_DLL_EXPORT
#define VTLOG_API __declspec(dllexport)
#else
#define VTLOG_API __declspec(dllimport)
#endif
