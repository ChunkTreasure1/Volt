#pragma once

#include <type_traits>

template<typename T>
concept Integer = std::is_integral<T>::value;

template<typename T>
concept Enum = std::is_enum_v<T>;

template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable<T>::value;
