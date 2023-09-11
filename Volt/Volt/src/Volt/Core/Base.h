#pragma once

#include "Weak.h"

#include <memory>
#include <string>
#include <iostream>

#define VT_VERSION Version::Create(0, 1, 1)

#define BIT(X) (1 << (X))
#define TO_NORMALIZEDRGB(r, g, b) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, 1.f }
#define TO_NORMALIZEDRGBA(r, g, b, a) glm::vec4{ r / 255.f, g / 255.f, b / 255.f, a / 255.f }

#define VT_PLATFROM_WINDOWS 1

const char* VKResultToString(int32_t result);

#define VT_ENABLE_GPU_MARKERS

#ifdef VT_DEBUG
#define VT_DEBUGBREAK() __debugbreak()
#define VT_ENABLE_DEBUG_ALLOCATIONS
#define VT_ENABLE_SHADER_DEBUG
#define VT_SHADER_PRINT
#define VT_PROFILE_GPU
#define VT_OPTIMIZE_ON
#define VT_OPTIMIZE_OFF

#define VT_VK_CHECK(x) if (x != VK_SUCCESS) { VT_CORE_ERROR("Vulkan Error: {0}", VKResultToString(x)); VT_DEBUGBREAK(); }

#else

#ifdef VT_DIST
#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
#define VT_OPTIMIZE_ON __pragma(optimize("", on));
#endif

#ifdef VT_RELEASE
#define VT_ENABLE_SHADER_DEBUG
#define VT_PROFILE_GPU
#define VT_OPTIMIZE_OFF __pragma(optimize("", off));
#define VT_OPTIMIZE_ON __pragma(optimize("", on));
#endif

#define VT_DEBUGBREAK() 
#define VT_VK_CHECK(x) x
#endif

#define SAFE_RELEASE(x) \
if (x)					\
{						\
	x->Release();		\
	x = nullptr;		\
}						\

#ifdef VT_ENABLE_ASSERTS
#define VT_ASSERT(x, ...) { if(!(x)) { VT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VT_DEBUGBREAK(); } }
#define VT_CORE_ASSERT(x, ...) { if(!(x)) { VT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); VT_DEBUGBREAK(); } }
#else
#define VT_ASSERT(x, ...)
#define VT_CORE_ASSERT(x, ...)
#endif

#define VT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

///// Helper Defines /////
#define VT_SETUP_ENUM_CLASS_OPERATORS(enumClass) \
inline enumClass operator|(enumClass aLhs, enumClass aRhs) \
{																						\
	return (enumClass)((std::underlying_type<enumClass>::type)aLhs | (std::underlying_type<enumClass>::type)aRhs); \
}\
\
inline enumClass operator&(enumClass aLhs, enumClass aRhs) \
{ \
	return (enumClass)((std::underlying_type<enumClass>::type)aLhs & (std::underlying_type<enumClass>::type)aRhs); \
} \
\
inline enumClass operator~(enumClass aLhs) \
{ \
	return (enumClass)(~(std::underlying_type<enumClass>::type)aLhs); \
}\
//////////////////////////

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

