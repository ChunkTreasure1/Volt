#pragma once
#include "type_vector2.h"
#include "type_vector3.h"
#include "type_vector4.h"

namespace gem
{
	template<uint32_t L, typename T>
	vec<L, bool> lessThan(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (uint32_t i = 0; i < L; ++i)
			result[i] = x[i] < y[i];
		return result;
	}

	template<uint32_t L, typename T>
	vec<L, bool> lessThanEqual(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (uint32_t i = 0; i < L; ++i)
			result[i] = x[i] <= y[i];
		return result;
	}

	template<uint32_t L, typename T>
	vec<L, bool> greaterThan(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (uint32_t i = 0; i < L; ++i)
			result[i] = x[i] > y[i];
		return result;
	}

	template<uint32_t L, typename T>
	vec<L, bool> greaterThanEqual(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (uint32_t i = 0; i < L; ++i)
			result[i] = x[i] >= y[i];
		return result;
	}

	template<uint32_t L, typename T>
	vec<L, bool> equal(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (uint32_t i = 0; i < L; ++i)
			result[i] = x[i] == y[i];
		return result;
	}

	template<uint32_t L, typename T>
	vec<L, bool> equal(vec<L, T> const& x, vec<L, T> const& y, vec<L, T> const& Epsilon)
	{
		return lessThanEqual(abs(x - y), Epsilon);
	}

	template<uint32_t L, typename T>
	vec<L, bool> equal(vec<L, T> const& x, vec<L, T> const& y, T Epsilon)
	{
		return equal(x, y, vec<L, T>(Epsilon));
	}

	template<uint32_t L, typename T>
	vec<L, bool> notEqual(vec<L, T> const& x, vec<L, T> const& y)
	{
		vec<L, bool> result(true);
		for (size_t i = 0; i < L; ++i)
			result[i] = x[i] != y[i];
		return result;
	}

	template<uint32_t L>
	bool any(vec<L, bool> const& v)
	{
		bool result = false;
		for (uint32_t i = 0; i < L; ++i)
			result = result || v[i];
		return result;
	}

	template<uint32_t L>
	bool all(vec<L, bool> const& v)
	{
		bool result = true;
		for (uint32_t i = 0; i < L; ++i)
			result = result && v[i];
		return result;
	}

	template<size_t L>
	vec<L, bool> not_(vec<L, bool> const& v)
	{
		bool result = true;
		for (uint32_t i = 0; i < L; ++i)
			result[i] = !v[i];
		return result;
	}
}