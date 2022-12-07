#pragma once
#include "vector/vector.h"
#include <cstdlib>

namespace gem
{
	template<typename T = float>
	T exp(const T& x)
	{
		return std::exp(x);
	};

	template<typename T = float>
	T exp2(const T& x)
	{
		return std::exp2(x);
	};

	template<typename T = float>
	T sqrt(const T& x)
	{
		return std::sqrt(x);
	};

	template<typename T = float>
	T inversesqrt(const T& x)
	{
		return 1 / sqrt(x);
	};

	template<typename T = float>
	T log(const T& x)
	{
		return std::log(x);
	};

	template<typename T = float>
	T log2(const T& x)
	{
		return std::log2(x);
	};

	template<typename T = float>
	T pow(const T& x, const T& exponent)
	{
		return std::pow(x, exponent);
	};

	// VECTORS

	template<size_t L, typename T = float>
	vec<L, T> exp(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(exp, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> exp2(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(exp2, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> sqrt(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(sqrt, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> inversesqrt(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(inversesqrt, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> log(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(log, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> log2(const vec<L, T>& x)
	{
		return functor1<vec, L, T, T>::call(log2, x);
	};

	template<size_t L, typename T = float>
	vec<L, T> pow(const vec<L, T>& x, const vec<L, T>& exponent)
	{
		return functor2<vec, L, T, T>::call(pow, x, exponent);
	};
}