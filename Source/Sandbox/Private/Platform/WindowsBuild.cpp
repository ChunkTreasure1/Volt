#include "sbpch.h"

#ifdef VT_PLATFORM_WINDOWS
extern "C" { __declspec(dllexport) extern const unsigned int D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif
