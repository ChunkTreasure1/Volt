#pragma once

#ifdef D3D12RHIMODULE_DLL_EXPORT
#define VTDX_API __declspec(dllexport)
#else
#define VTDX_API __declspec(dllimport)
#endif
