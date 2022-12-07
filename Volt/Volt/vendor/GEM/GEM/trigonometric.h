#pragma once
#include "vector/vector.h"
#include <cstdlib>
#include <cassert>

// MAGIC NUMBERS COPIED FROM GLM

namespace gem
{
	template<typename T = float>
	T radians(const T& degrees)
	{
		assert(std::numeric_limits<T>::is_iec559 && "only accepts floating-point inputs");
		return (degrees * T(0.01745329251994329576923690768489));
	};

	template<typename T = float>
	T degrees(const T& radians)
	{
		assert(std::numeric_limits<T>::is_iec559 && "only accepts floating-point inputs");
		return (radians * T(57.295779513082320876798154814105));
	};

	template<typename T = float>
	T cos(const T& v)
	{
		return std::cos(v);
	};

	template<typename T = float>
	T acos(const T& v)
	{
		return std::acos(v);
	};

	template<typename T = float>
	T cosh(const T& v)
	{
		return std::cosh(v);
	};

	template<typename T = float>
	T acosh(const T& v)
	{
		return std::acosh(v);
	};

	template<typename T = float>
	T sin(const T& v)
	{
		return std::sin(v);
	};

	template<typename T = float>
	T asin(const T& v)
	{
		return std::asin(v);
	};

	template<typename T = float>
	T sinh(const T& v)
	{
		return std::sinh(v);
	};

	template<typename T = float>
	T asinh(const T& v)
	{
		return std::asinh(v);
	};

	template<typename T = float>
	T tan(const T& v)
	{
		return std::tan(v);
	};

	template<typename T = float>
	T atan(const T& v)
	{
		return std::atan(v);
	};

	template<typename T = float>
	T tanh(const T& v)
	{
		return std::tanh(v);
	};

	template<typename T = float>
	T atanh(const T& v)
	{
		return std::atanh(v);
	};

	template<typename T>
	T atan(T y, T x)
	{
		assert(std::numeric_limits<T>::is_iec559 && "'atan' only accept floating-point input");

		return ::std::atan2(y, x);
	}

	// VECTORS

	template<size_t L, typename T = float>
	vec<L, T> radians(const vec<L, T>& degrees)
	{
		return functor1<vec, L, T, T>::call(radians, degrees);
	};

	template<size_t L, typename T = float>
	vec<L, T> degrees(const vec<L, T>& radians)
	{
		return functor1<vec, L, T, T>::call(degrees, radians);
	};

	template<size_t L, typename T = float>
	vec<L, T> cos(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(cos, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> acos(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(acos, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> cosh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(cosh, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> acosh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(acosh, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> sin(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(sin, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> asin(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(asin, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> sinh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(sinh, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> asinh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(asinh, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> tan(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(tan, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> atan(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(atan, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> tanh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(tanh, v);
	};

	template<size_t L, typename T = float>
	vec<L, T> atanh(const vec<L, T>& v)
	{
		return functor1<vec, L, T, T>::call(atanh, v);
	};
}