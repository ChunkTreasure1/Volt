#pragma once
#include "vector/vector.h"
#include "matrix/matrix.h"

namespace gem
{
	template<typename T>
	T* const* value_ptr(T const& v)
	{
		return &(v);
	};

	template<typename T>
	T* value_ptr(T& v)
	{
		return &(v);
	};

	template<typename T>
	T const* value_ptr(vec<2, T> const& v)
	{
		return &(v.x);
	};

	template<typename T>
	T* value_ptr(vec<2, T>& v)
	{
		return &(v.x);
	};

	template<typename T>
	T const* value_ptr(vec<3, T> const& v)
	{
		return &(v.x);
	};

	template<typename T>
	T* value_ptr(vec<3, T>& v)
	{
		return &(v.x);
	};

	template<typename T>
	T const* value_ptr(vec<4, T> const& v)
	{
		return &(v.x);
	};

	template<typename T>
	T* value_ptr(vec<4, T>& v)
	{
		return &(v.x);
	};

	template<typename T>
	T const* value_ptr(mat<2, 2, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<2, 2, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<3, 3, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<3, 3, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<4, 4, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<4, 4, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<2, 3, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<2, 3, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<3, 2, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<3, 2, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<2, 4, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<2, 4, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<4, 2, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<4, 2, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<3, 4, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<3, 4, T>& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T const* value_ptr(mat<4, 3, T> const& m)
	{
		return &(m[0].x);
	};

	template<typename T>
	T* value_ptr(mat<4, 3, T>& m)
	{
		return &(m[0].x);
	};
}