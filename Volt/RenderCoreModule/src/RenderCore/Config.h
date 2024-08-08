#pragma once

#ifdef VTRC_BUILD_DLL
#define VTRC_API __declspec(dllexport)
#else
#define VTRC_API __declspec(dllimport)
#endif
