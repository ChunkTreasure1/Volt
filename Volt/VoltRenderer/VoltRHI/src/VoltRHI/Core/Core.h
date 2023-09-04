#pragma once

#include <Volt/Core/Weak.h>

#include <memory>
#include <string>

#define VT_NODISCARD [[nodiscard]]
#define VT_INLINE __forceinline
#define VT_DELETE_COMMON_OPERATORS(X) X(const X&) = delete; X& operator=(const X&) = delete; X(X&&) = delete; X& operator=(X&&) = delete

///// Helper Defines /////
#define VT_SETUP_ENUM_CLASS_OPERATORS(enumType) \
	inline           enumType& operator|=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((__underlying_type(enumType))Lhs | (__underlying_type(enumType))Rhs); } \
	inline           enumType& operator&=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((__underlying_type(enumType))Lhs & (__underlying_type(enumType))Rhs); } \
	inline           enumType& operator^=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((__underlying_type(enumType))Lhs ^ (__underlying_type(enumType))Rhs); } \
	inline constexpr enumType  operator| (enumType  Lhs, enumType Rhs) { return (enumType)((__underlying_type(enumType))Lhs | (__underlying_type(enumType))Rhs); } \
	inline constexpr enumType  operator& (enumType  Lhs, enumType Rhs) { return (enumType)((__underlying_type(enumType))Lhs & (__underlying_type(enumType))Rhs); } \
	inline constexpr enumType  operator^ (enumType  Lhs, enumType Rhs) { return (enumType)((__underlying_type(enumType))Lhs ^ (__underlying_type(enumType))Rhs); } \
	inline constexpr bool  operator! (enumType  E)             { return !(__underlying_type(enumType))E; } \
	inline constexpr enumType  operator~ (enumType  E)             { return (enumType)~(__underlying_type(enumType))E; }

#define BIT(X) (1 << (X))
//////////////////////////

#ifdef VT_PLATFORM_WINDOWS
#define VT_RHI_DEBUGBREAK() __debugbreak();
#else
#error "Debug break not implemented!"
#endif

#ifdef VT_DEBUG

#define VT_ENABLE_GPU_MARKERS
#define VT_ENABLE_DEBUG_ALLOCATIONS

#elif VT_RELEASE

#define VT_ENABLE_GPU_MARKERS

#elif VT_DIST

#endif

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename ... Args>
constexpr Scope<T> CreateScopeRHI(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRefRHI(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
