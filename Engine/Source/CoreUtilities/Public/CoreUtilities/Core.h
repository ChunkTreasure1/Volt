#pragma once

#include "Weak.h"
#include "Config.h"
#include "CompilerTraits.h"

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

#define VT_DELETE_COPY_MOVE(X) X(const X&) = delete; X& operator=(const X&) = delete; X(X&&) = delete; X& operator=(X&&) = delete
#define VT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
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
