#pragma once

namespace gem
{
	// Functor1

	template<template<size_t L, typename T> class vec, size_t L, typename R, typename T>
	struct functor1 {};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor1<vec, 1, R, T>
	{
		static vec<1, R> call(R(*func)(const T& x), const vec<1, T>& a)
		{
			return vec<1, R>(func(a.x));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor1<vec, 2, R, T>
	{
		static vec<2, R> call(R(*func)(const T& x), const vec<2, T>& a)
		{
			return vec<2, R>(func(a.x), func(a.y));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor1<vec, 3, R, T>
	{
		static vec<3, R> call(R(*func)(const T& x), const vec<3, T>& a)
		{
			return vec<3, R>(func(a.x), func(a.y), func(a.z));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor1<vec, 4, R, T>
	{
		static vec<4, R> call(R(*func)(const T& x), const vec<4, T>& a)
		{
			return vec<4, R>(func(a.x), func(a.y), func(a.z), func(a.w));
		}
	};

	// Functor2

	template<template<size_t L, typename T> class vec, size_t L, typename R, typename T>
	struct functor2 {};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor2<vec, 1, R, T>
	{
		static vec<1, R> call(R(*func)(const T& x, const T& y), const vec<1, T>& a, const vec<1, T>& b)
		{
			return vec<1, R>(func(a.x, b.x));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor2<vec, 2, R, T>
	{
		static vec<2, R> call(R(*func)(const T& x, const T& y), const vec<2, T>& a, const vec<2, T>& b)
		{
			return vec<2, R>(func(a.x, b.x), func(a.y, b.y));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor2<vec, 3, R, T>
	{
		static vec<3, R> call(R(*func)(const T& x, const T& y), const vec<3, T>& a, const vec<3, T>& b)
		{
			return vec<3, R>(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z));
		}
	};

	template<template<size_t L, typename T> class vec, typename R, typename T>
	struct functor2<vec, 4, R, T>
	{
		static vec<4, R> call(R(*func)(const T& x, const T& y), const vec<4, T>& a, const vec<4, T>& b)
		{
			return vec<4, R>(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z), func(a.w, b.w));
		}
	};
}