#pragma once

#ifdef VTDX_BUILD_DLL
#define VTDX_API __declspec(dllexport)
#else
#define VTDX_API __declspec(dllimport)
#endif
