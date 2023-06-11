#pragma once
#include "vector/vector.h"
#include <cstdlib>
#include <cassert>

namespace gem
{
	template<size_t L, typename T = float>
	T distance(const vec<L, T>& x, const vec<L, T>& y)
	{
		return length(x - y);
	};

	template<size_t L, typename T = float>
	T distance2(const vec<L, T>& x, const vec<L, T>& y)
	{
		return length2(x - y);
	};

	template<size_t L, typename T = float>
	T dot(const vec<2, T>& x, const vec<L, T>& y)
	{
		vec<2, T> result(x * y);
		return result.x + result.y;
	};

	template<size_t L, typename T = float>
	T dot(const vec<3, T>& x, const vec<L, T>& y)
	{
		vec<3, T> result(x * y);
		return result.x + result.y + result.z;
	};

	template<size_t L, typename T = float>
	T dot(const vec<4, T>& x, const vec<L, T>& y)
	{
		vec<4, T> result(x * y);
		return result.x + result.y + result.z + result.w;
	};

	template<typename T = float>
	T dot(const qua<T>& a, const qua<T>& b)
	{
		vec<4, T> tmp(a.w * b.w, a.x * b.x, a.y * b.y, a.z * b.z);
		return (tmp.x + tmp.y) + (tmp.z + tmp.w);
	};

	template<size_t L, typename T = float>
	T length(const vec<L, T>& x)
	{
		return sqrt(dot(x, x));
	};

	template<typename T>
	inline T length(qua<T> const& q)
	{
		return sqrt(dot(q, q));
	}

	template<typename T = float>
	T lerp(T a, T b, float c)
	{
		return (static_cast<T>(1) - c) * a + c * b;
	}

	template<typename T = float>
	vec<3, T> cross(const vec<3, T>& x, const vec<3, T>& y)
	{
		assert(std::numeric_limits<T>::is_iec559 && "only accepts floating-point inputs");
		return vec<3, T>(
			x.y * y.z - y.y * x.z,
			x.z * y.x - y.z * x.x,
			x.x * y.y - y.x * x.y);
	};

	template<size_t L, typename T = float>
	vec<L, T> normalize(const vec<L, T>& x)
	{
		assert(std::numeric_limits<T>::is_iec559 && "only accepts floating-point inputs");
		return x * inversesqrt(dot(x, x));
	};

	template<typename T>
	inline qua<T> normalize(qua<T> const& q)
	{
		T len = length(q);
		if (len <= static_cast<T>(0)) // Problem
			return qua<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));
		T oneOverLen = static_cast<T>(1) / len;
		return qua<T>(q.w * oneOverLen, q.x * oneOverLen, q.y * oneOverLen, q.z * oneOverLen);
	}

	template<typename T = float>
	T length2(vec<3, T> const& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	template<typename T = float>
	T length2(vec<2, T> const& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	template<typename T = float>
	T nlerp(T a, T b, float c)
	{
		T val = lerp(a, b, c);

		if (length(val) != 0.f)
		{
			val = normalize(val);
		}

		return val;
	}

	template <typename T>
	qua<T> slerp(const qua<T>& x, const qua<T>& y, T a)
	{
		qua<T> z = y;
		qua<T> w = x;

		// dot product - the cosine of the angle between the two quaternions
		T cosTheta = dot(w, z);

		w = normalize(w);
		z = normalize(z);

		// make sure the interpolation factor is in the range [0, 1]
		a = std::clamp(a, static_cast<T>(0), static_cast<T>(1));

		// negate one of the quaternions if they are negated to each other
		if (cosTheta < 0)
		{
			z = -z;
			cosTheta = -cosTheta;
		}

		// use lerp if the quaternions are collinear
		if (std::abs(cosTheta) > 1 - 1e-3)
		{
			return glm::normalize(qua<T>(
				lerp(w.w, z.w, a),
				lerp(w.x, z.x, a),
				lerp(w.y, z.y, a),
				lerp(w.z, z.z, a)));
		}

		// calculate the angle between the two quaternions
		T angle = std::acos(cosTheta);

		// calculate the interpolation factors
		T s1 = std::sin((1 - a) * angle);
		T s2 = std::sin(a * angle);

		// interpolate and return the result
		return (s1 * w + s2 * z) / std::sin(angle);
	}

	template<size_t L, typename T>
	inline vec<L, T> fract(const vec<L, T>& x)
	{
		return x - floor(x);
	}

	template<size_t L, typename T>
	inline vec<L, T> mod(const vec<L, T>& x, T y)
	{
		return x - y * floor(x / y);
	}

	template<size_t L, typename T>
	inline vec<L, T> mod(const vec<L, T>& x, const vec<L, T>& y)
	{
		return x - y * floor(x / y);
	}
}
