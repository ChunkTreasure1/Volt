#pragma once

#include <CoreUtilities/Core.h>

#include <memory>
#include <string>
#include <iostream>

#define VT_VERSION Version::Create(0, 1, 3)

#define TO_NORMALIZEDRGB(r, g, b) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, 1.f }
#define TO_NORMALIZEDRGBA(r, g, b, a) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, a / 255.f }

#define VT_PLATFROM_WINDOWS 1

#define VT_ENABLE_GPU_MARKERS

#ifdef VT_DEBUG
#define VT_DEBUGBREAK() __debugbreak()
#define VT_ENABLE_DEBUG_ALLOCATIONS
#define VT_ENABLE_SHADER_DEBUG
#define VT_SHADER_PRINT
#define VT_PROFILE_GPU
#define VT_OPTIMIZE_ON
#define VT_OPTIMIZE_OFF

#else

#ifdef VT_RELEASE
#define VT_ENABLE_SHADER_DEBUG
#define VT_PROFILE_GPU
#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
#define VT_OPTIMIZE_ON __pragma(optimize("", on));

#endif

#ifdef VT_DIST
#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
#define VT_OPTIMIZE_ON __pragma(optimize("", on));
#endif

#define VT_DEBUGBREAK() 
#endif

#ifdef VT_ENABLE_ASSERTS
#define VT_ASSERT(x, ...) { if(!(x)) { VT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VT_DEBUGBREAK(); } }
#define VT_CORE_ASSERT(x, ...) { if(!(x)) { VT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VT_DEBUGBREAK(); } }
#else
#define VT_ASSERT(x, ...)
#define VT_CORE_ASSERT(x, ...)
#endif

#define VT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
