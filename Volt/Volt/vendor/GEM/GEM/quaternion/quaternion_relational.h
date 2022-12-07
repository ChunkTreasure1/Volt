#pragma once
#include "type_quaternion.h"

namespace gem
{
	template<typename T>
	vec<4, bool> equal(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (size_t i = 0; i < x.length(); ++i)
			result[i] = x[i] == y[i];
		return result;
	}

	template<typename T>
	vec<4, bool> equal(qua<T> const& x, qua<T> const& y, T epsilon)
	{
		vec<4, T> v(x.x - y.x, x.y - y.y, x.z - y.z, x.w - y.w);
		return lessThan(abs(v), vec<4, T>(epsilon));
	}

	template<typename T>
	vec<4, bool> notEqual(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (size_t i = 0; i < x.length(); ++i)
			result[i] = x[i] != y[i];
		return result;
	}

	template<typename T>
	vec<4, bool> notEqual(qua<T> const& x, qua<T> const& y, T epsilon)
	{
		vec<4, T> v(x.x - y.x, x.y - y.y, x.z - y.z, x.w - y.w);
		return greaterThanEqual(abs(v), vec<4, T>(epsilon));
	}
}