#pragma once

template<typename T, typename... Args>
inline constexpr bool IsEqualToAny(const T& value, Args&&... args)
{
	return ((value == args) || ...);
}

template<typename T, typename... Args>
inline constexpr bool IsEqualToAll(const T& value, Args&&... args)
{
	return ((value == args) && ...);
}
