#pragma once

#ifdef LOGMODULE_BUILD_DLL
#define VTLOG_API __declspec(dllexport)
#else
#define VTLOG_API __declspec(dllimport)
#endif
