#pragma once

#include "common.h"

namespace gem
{
	template<typename T = float>
	bool epsilonEqual
	(
		T const& x,
		T const& y,
		T const& epsilon
	)
	{
		return abs(x - y) < epsilon;
	}

	template<typename T>
	bool epsilonNotEqual(T const& x, T const& y, T const& epsilon)
	{
		return abs(x - y) >= epsilon;
	}

	template<size_t L, typename T>
	vec<L, bool> epsilonEqual(vec<L, T> const& x, vec<L, T> const& y, T const& epsilon)
	{
		return lessThan(abs(x - y), vec<L, T>(epsilon));
	}

	template<size_t L, typename T>
	vec<L, bool> epsilonEqual(vec<L, T> const& x, vec<L, T> const& y, vec<L, T> const& epsilon)
	{
		return lessThan(abs(x - y), vec<L, T>(epsilon));
	}

	template<size_t L, typename T>
	vec<L, bool> epsilonNotEqual(vec<L, T> const& x, vec<L, T> const& y, T const& epsilon)
	{
		return greaterThanEqual(abs(x - y), vec<L, T>(epsilon));
	}

	template<size_t L, typename T>
	vec<L, bool> epsilonNotEqual(vec<L, T> const& x, vec<L, T> const& y, vec<L, T> const& epsilon)
	{
		return greaterThanEqual(abs(x - y), vec<L, T>(epsilon));
	}

	template<typename T>
	vec<4, bool> epsilonEqual(qua<T> const& x, qua<T> const& y, T const& epsilon)
	{
		vec<4, T> v(x.x - y.x, x.y - y.y, x.z - y.z, x.w - y.w);
		return lessThan(abs(v), vec<4, T>(epsilon));
	}

	template<typename T>
	vec<4, bool> epsilonNotEqual(qua<T> const& x, qua<T> const& y, T const& epsilon)
	{
		vec<4, T> v(x.x - y.x, x.y - y.y, x.z - y.z, x.w - y.w);
		return greaterThanEqual(abs(v), vec<4, T>(epsilon));
	}
}