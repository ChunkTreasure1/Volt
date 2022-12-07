#pragma once

#include "geometric.h"

namespace gem
{
	template<typename T>
	inline T mod289(T const& x)
	{
		return x - floor(x * (static_cast<T>(1.0) / static_cast<T>(289.0))) * static_cast<T>(289.0);
	}

	template<typename T>
	inline T permute(T const& x)
	{
		return mod289(((x * static_cast<T>(34)) + static_cast<T>(1)) * x);
	}

	template<typename T>
	inline vec<2, T> permute(vec<2, T> const& x)
	{
		return mod289(((x * static_cast<T>(34)) + static_cast<T>(1)) * x);
	}

	template<typename T>
	inline vec<3, T> permute(vec<3, T> const& x)
	{
		return mod289(((x * static_cast<T>(34)) + static_cast<T>(1)) * x);
	}

	template<typename T>
	inline vec<4, T> permute(vec<4, T> const& x)
	{
		return mod289(((x * static_cast<T>(34)) + static_cast<T>(1)) * x);
	}

	template<typename T>
	inline T taylorInvSqrt(T const& r)
	{
		return static_cast<T>(1.79284291400159) - static_cast<T>(0.85373472095314) * r;
	}

	template<typename T>
	inline vec<2, T> taylorInvSqrt(vec<2, T> const& r)
	{
		return static_cast<T>(1.79284291400159) - static_cast<T>(0.85373472095314) * r;
	}

	template<typename T>
	inline vec<3, T> taylorInvSqrt(vec<3, T> const& r)
	{
		return static_cast<T>(1.79284291400159) - static_cast<T>(0.85373472095314) * r;
	}

	template<typename T>
	inline vec<4, T> taylorInvSqrt(vec<4, T> const& r)
	{
		return static_cast<T>(1.79284291400159) - static_cast<T>(0.85373472095314) * r;
	}

	template<typename T>
	inline vec<2, T> fade(vec<2, T> const& t)
	{
		return (t * t * t) * (t * (t * static_cast<T>(6) - static_cast<T>(15)) + static_cast<T>(10));
	}

	template<typename T>
	inline vec<3, T> fade(vec<3, T> const& t)
	{
		return (t * t * t) * (t * (t * static_cast<T>(6) - static_cast<T>(15)) + static_cast<T>(10));
	}

	template<typename T>
	inline vec<4, T> fade(vec<4, T> const& t)
	{
		return (t * t * t) * (t * (t * static_cast<T>(6) - static_cast<T>(15)) + static_cast<T>(10));
	}

	template<size_t L, typename T = float>
	T perlin(const vec<L, T>& p);

	template<size_t L, typename T = float>
	T perlin(const vec<L, T>& p, const vec<L, T>& rep);

	template<size_t L, typename T = float>
	T simplex(const vec<L, T>& p);

	template<typename T>
	T perlin(const vec<2, T>& position)
	{
		//vec<4, T> Pi = gem::floor(vec<4, T>(position.x, position.y, position.x, position.y)) + vec<4, T>(0.0, 0.0, 1.0, 1.0);
		//vec<4, T> Pf = gem::fract(vec<4, T>(position.x, position.y, position.x, position.y)) - vec<4, T>(0.0, 0.0, 1.0, 1.0);
		//Pi = gem::mod(Pi, vec<4, T>(289)); // To avoid truncation effects in permutation
		//vec<4, T> ix(Pi.x, Pi.z, Pi.x, Pi.z);
		//vec<4, T> iy(Pi.y, Pi.y, Pi.w, Pi.w);
		//vec<4, T> fx(Pf.x, Pf.z, Pf.x, Pf.z);
		//vec<4, T> fy(Pf.y, Pf.y, Pf.w, Pf.w);

		//vec<4, T> i = permute(permute(ix) + iy);

		//vec<4, T> gx = gem::fract(i / T(41)) * static_cast<T>(2) - T(1);
		//vec<4, T> gy = gem::abs(gx) - T(0.5);
		//vec<4, T> tx = gem::floor(gx + T(0.5));
		//gx = gx - tx;

		//vec<2, T> g00(gx.x, gy.x);
		//vec<2, T> g10(gx.y, gy.y);
		//vec<2, T> g01(gx.z, gy.z);
		//vec<2, T> g11(gx.w, gy.w);

		//vec<4, T> norm = taylorInvSqrt(vec<4, T>(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
		//g00 *= norm.x;
		//g01 *= norm.y;
		//g10 *= norm.z;
		//g11 *= norm.w;

		//T n00 = dot(g00, vec<2, T>(fx.x, fy.x));
		//T n10 = dot(g10, vec<2, T>(fx.y, fy.y));
		//T n01 = dot(g01, vec<2, T>(fx.z, fy.z));
		//T n11 = dot(g11, vec<2, T>(fx.w, fy.w));

		//vec<2, T> fade_xy = fade(vec<2, T>(Pf.x, Pf.y));
		//vec<2, T> n_x = lerp(vec<2, T>(n00, n01), vec<2, T>(n10, n11), fade_xy.x);
		//T n_xy = lerp(n_x.x, n_x.y, fade_xy.y);
		return T(2.3);
	}
}