#pragma once
#include "type_matrix3.h"
#include "type_matrix4.h"

namespace gem
{
	template<size_t C, size_t R, typename T>
	vec<C, bool> equal(mat<C, R, T> const& x, mat<C, R, T> const& y)
	{

	}

	template<size_t C, size_t R, typename T>
	vec<C, bool> notEqual(mat<C, R, T> const& x, mat<C, R, T> const& y);

	template<size_t C, size_t R, typename T>
	vec<C, bool> equal(mat<C, R, T> const& x, mat<C, R, T> const& y, T epsilon);

	template<size_t C, size_t R, typename T>
	vec<C, bool> equal(mat<C, R, T> const& x, mat<C, R, T> const& y, vec<C, T> const& epsilon);

	template<size_t C, size_t R, typename T>
	vec<C, bool> notEqual(mat<C, R, T> const& x, mat<C, R, T> const& y, T epsilon);

	template<size_t C, size_t R, typename T>
	vec<C, bool> notEqual(mat<C, R, T> const& x, mat<C, R, T> const& y, vec<C, T> const& epsilon);

	template<size_t C, size_t R, typename T>
	vec<C, bool> equal(mat<C, R, T> const& x, mat<C, R, T> const& y, int ULPs);

	template<size_t C, size_t R, typename T>
	vec<C, bool> equal(mat<C, R, T> const& x, mat<C, R, T> const& y, vec<C, int> const& ULPs);

	template<size_t C, size_t R, typename T>
	vec<C, bool> notEqual(mat<C, R, T> const& x, mat<C, R, T> const& y, int ULPs);

	template<size_t C, size_t R, typename T>
	vec<C, bool> notEqual(mat<C, R, T> const& x, mat<C, R, T> const& y, vec<C, int> const& ULPs);
}