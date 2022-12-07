#pragma once
#include "vector/vector.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace gem
{
	template<typename T = float>
	T abs(const T& a)
	{
		return std::abs(a);
	};

	template<typename T = float>
	T ceil(const T& a)
	{
		return std::ceil(a);
	}

	template<typename T = float>
	T floor(const T& a)
	{
		return std::floor(a);
	}

	template<typename T = float>
	T clamp(const T& a, const T& minV, const T& maxV)
	{
		return std::clamp(a, minV, maxV);
	}

	template<typename T = float>
	T min(const T& a, const T& b)
	{
		return std::min(a, b);
	}

	template<typename T = float>
	T max(const T& a, const T& b)
	{
		return std::max(a, b);
	}

	// VECTORS

	template<size_t L, typename T = float>
	vec<L, T> abs(const vec<L, T>& a)
	{
		return functor1<vec, L, T, T>::call(abs, a);
	};

	template<size_t L, typename T = float>
	vec<L, T> ceil(const vec<L, T>& a)
	{
		return functor1<vec, L, T, T>::call(ceil, a);
	}

	template<size_t L, typename T = float>
	vec<L, T> floor(const vec<L, T>& a)
	{
		return functor1<vec, L, T, T>::call(floor, a);
	}

	template<size_t L, typename T = float>
	vec<L, T> clamp(const vec<L, T>& a, const vec<L, T>& minV, const vec<L, T>& maxV)
	{
		return min(max(a, minV), maxV);
	}

	template<size_t L, typename T = float>
	vec<L, T> clamp(const vec<L, T>& a, const T& minV, const T& maxV)
	{
		return min(max(a, minV), maxV);
	}

	template<size_t L, typename T = float>
	vec<L, T> min(const vec<L, T>& a, const vec<L, T>& b)
	{
		return functor2<vec, L, T, T>::call(min, a, b);
	}

	template<size_t L, typename T = float>
	vec<L, T> min(const vec<L, T>& a, const T& b)
	{
		return functor2<vec, L, T, T>::call(min, a, vec<L, T>(b));
	}

	template<size_t L, typename T = float>
	vec<L, T> max(const vec<L, T>& a, const vec<L, T>& b)
	{
		return functor2<vec, L, T, T>::call(max, a, b);
	}

	template<size_t L, typename T = float>
	vec<L, T> max(const vec<L, T>& a, const T& b)
	{
		return functor2<vec, L, T, T>::call(max, a, vec<L, T>(b));
	}
}