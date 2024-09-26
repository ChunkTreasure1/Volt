#pragma once

#include <type_traits>

template<typename, typename = std::void_t<>>
struct HasEquality : std::false_type {};

template<typename T>
struct HasEquality<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type
{ };

template<typename T>
constexpr auto HasEqualityV = HasEquality<T>::value;
