#pragma once

#ifdef VOLT_ASSETS_DLL_EXPORT
#define VTASSETS_API __declspec(dllexport)
#else
#define VTASSETS_API __declspec(dllimport)
#endif
