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
	T dot(const qua<T>& x, const qua<T>& y)
	{
		qua<T> result(x * y);
		return result.x + result.y + result.z + result.w;
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
		return x * inversesqrt(dot(x, x));;
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

	template<typename T = float>
	qua<T> slerp(const qua<T>& x, const qua<T>& y, T a)
	{
		qua<T> z = y;

		T cosTheta = dot(x, y);

		if (cosTheta < static_cast<T>(0))
		{
			z = -y;
			cosTheta = -cosTheta;
		}

		if (cosTheta > static_cast<T>(1) - epsilon<T>())
		{
			return qua<T>(
				lerp(x.w, z.w, a),
				lerp(x.x, z.x, a),
				lerp(x.y, z.y, a),
				lerp(x.z, z.z, a));
		}

		T angle = acos(cosTheta);
		return (sin((static_cast<T>(1) - a) * angle) * x + sin(a * angle) * z) / sin(angle);
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