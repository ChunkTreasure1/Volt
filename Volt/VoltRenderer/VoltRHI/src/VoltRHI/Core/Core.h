#pragma once

#include <memory>
#include <string>

#define VT_NODISCARD [[nodiscard]]
#define VT_INLINE __forceinline
#define VT_DELETE_COMMON_OPERATORS(X) X(const X&) = delete; X& operator=(const X&) = delete; X(X&&) = delete; X& operator=(X&&) = delete

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

template<typename T>
using Weak = std::weak_ptr<T>;
