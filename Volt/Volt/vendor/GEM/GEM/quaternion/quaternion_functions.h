#pragma once

#include "../constants.h"
#include "../exponential.h"
#include "../trigonometric.h"
#include "../geometric.h"
#include "../vector/vector.h"
#include "../vector/vector_relational.h"
#include "../matrix/matrix.h"
#include "../matrix/matrix_transform.h"
#include "type_quaternion.h"
#include "quaternion_relational.h"

#ifndef GEM_FORCE_RIGHT_HANDED
#define GEM_LEFT_HANDED
#endif

namespace gem
{
	template<typename T>
	T roll(qua<T> const& q)
	{
		return static_cast<T>(atan(static_cast<T>(2) * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z));
	}

	template<typename T>
	T pitch(qua<T> const& q)
	{
		T const y = static_cast<T>(2) * (q.y * q.z + q.w * q.x);
		T const x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;

		if (all(equal(vec<2, T>(x, y), vec<2, T>(0), epsilon<T>())))
			return static_cast<T>(static_cast<T>(2) * atan(q.x, q.w));

		return static_cast<T>(atan(y, x));
	}

	template<typename T>
	T yaw(qua<T> const& q)
	{
		return asin(clamp(static_cast<T>(-2) * (q.x * q.z - q.w * q.y), static_cast<T>(-1), static_cast<T>(1)));
	}

	template<typename T>
	vec<3, T> eulerAngles(qua<T> const& x)
	{
		return vec<3, T>(pitch(x), yaw(x), roll(x));
	}

	template<typename T>
	mat<3, 3, T> mat3_cast(qua<T> const& q)
	{
		mat<3, 3, T> result(T(1));
		T qxx(q.x * q.x);
		T qyy(q.y * q.y);
		T qzz(q.z * q.z);
		T qxz(q.x * q.z);
		T qxy(q.x * q.y);
		T qyz(q.y * q.z);
		T qwx(q.w * q.x);
		T qwy(q.w * q.y);
		T qwz(q.w * q.z);

		result[0][0] = T(1) - T(2) * (qyy + qzz);
		result[0][1] = T(2) * (qxy + qwz);
		result[0][2] = T(2) * (qxz - qwy);

		result[1][0] = T(2) * (qxy - qwz);
		result[1][1] = T(1) - T(2) * (qxx + qzz);
		result[1][2] = T(2) * (qyz + qwx);

		result[2][0] = T(2) * (qxz + qwy);
		result[2][1] = T(2) * (qyz - qwx);
		result[2][2] = T(1) - T(2) * (qxx + qyy);
		return result;
	}

	template<typename T>
	mat<4, 4, T> mat4_cast(qua<T> const& q)
	{
		return mat<4, 4, T>(mat3_cast(q));
	}

	template<typename T>
	qua<T> quat_cast(mat<3, 3, T> const& m)
	{
		T fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
		T fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
		T fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
		T fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

		int biggestIndex = 0;
		T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
		T mult = static_cast<T>(0.25) / biggestVal;

		switch (biggestIndex)
		{
		case 0:
			return qua<T>(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);

		case 1:
			return qua<T>((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);

		case 2:
			return qua<T>((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);

		case 3:
			return qua<T>((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);

		default:
			assert(false);
			return qua<T>(1, 0, 0, 0);
		}
	}

	template<typename T>
	qua<T> quat_cast(mat<4, 4, T> const& m)
	{
		return quat_cast(mat<3, 3, T>(m));
	}

	template<typename T>
	vec<4, bool> lessThan(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (int i = 0; i < x.length(); ++i)
			result[i] = x[i] < y[i];
		return result;
	}

	template<typename T>
	vec<4, bool> lessThanEqual(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (int i = 0; i < x.length(); ++i)
			result[i] = x[i] <= y[i];
		return result;
	}

	template<typename T>
	vec<4, bool> greaterThan(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (int i = 0; i < x.length(); ++i)
			result[i] = x[i] > y[i];
		return result;
	}

	template<typename T>
	vec<4, bool> greaterThanEqual(qua<T> const& x, qua<T> const& y)
	{
		vec<4, bool> result;
		for (int i = 0; i < x.length(); ++i)
			result[i] = x[i] >= y[i];
		return result;
	}

	template<typename T>
	qua<T> quatLookAtRH(
		vec<3, T> const& direction,
		vec<3, T> const& up)
	{
		mat<3, 3, T> result;

		result[2] = -direction;
		vec<3, T> const& Right = cross(up, result[2]);
		result[0] = Right * inversesqrt(max(static_cast<T>(0.00001), dot(Right, Right)));
		result[1] = cross(result[2], result[0]);

		return quat_cast(result);
	}

	template<typename T>
	qua<T> quatLookAtLH(
		vec<3, T> const& direction,
		vec<3, T> const& up)
	{
		mat<3, 3, T> result;

		result[2] = direction;
		vec<3, T> const Right = cross(up, result[2]);
		result[0] = Right * inversesqrt(max(static_cast<T>(0.00001), dot(Right, Right)));
		result[1] = cross(result[2], result[0]);

		return quat_cast(result);
	}

	template<typename T>
	qua<T> quatLookAt(
		vec<3, T> const& direction,
		vec<3, T> const& up)
	{
#ifdef GEM_LEFT_HANDED
		return quatLookAtLH(direction, up);
#else
		return quatLookAtRH(direction, up);
#endif
	}

	template<typename T>
	T length2(qua<T> const& q)
	{
		return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	}

	template<typename T>
	qua<T> angleAxis(T const& angle, vec<3, T> const& v)
	{
		T const a(angle);
		T const s = glm::sin(a * static_cast<T>(0.5));

		return qua<T>(glm::cos(a * static_cast<T>(0.5f)), v * s);
	}

	template<typename T>
	qua<T> fromTo(vec<3, T> const& from, vec<3, T> const& to)
	{
		T const cosTheta = dot(from, to);
		vec<3, T> const& rotationAxis = cross(from, to);

		return qua<T>(cosTheta, rotationAxis);
	}

	/// Compute the rotation between two vectors.
	/// @param orig vector, needs to be normalized
	/// @param dest vector, needs to be normalized
	template<typename T>
	qua<T> rotation(vec<3, T> const& origin, vec<3, T> const& destination)
	{
		T cosTheta = dot(origin, destination);
		vec<3, T> rotationAxis;

		if (cosTheta >= static_cast<T>(1) - epsilon<T>())
		{
			return { static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) };
		}

		if (cosTheta < static_cast<T>(-1) + epsilon<T>())
		{
			// special case when vectors in opposite directions :
			// there is no "ideal" rotation axis
			// So guess one; any will do as long as it's perpendicular to start
			// This implementation favors a rotation around the Up axis (Y),
			// since it's often what you want to do.
			rotationAxis = cross(vec<3, T>(0, 0, 1), origin);
			if (length2(rotationAxis) < epsilon<T>()) // bad luck, they were parallel, try again!
			{
				rotationAxis = cross(vec<3, T>(1, 0, 0), origin);
			}

			rotationAxis = normalize(rotationAxis);
			return angleAxis(pi<T>(), rotationAxis);
		}

		// Implementation from Stan Melax's Game Programming Gems 1 article
		rotationAxis = cross(origin, destination);

		T s = sqrt((T(1) + cosTheta) * static_cast<T>(2));
		T invs = static_cast<T>(1) / s;

		return qua<T>(
			s * static_cast<T>(0.5f),
			rotationAxis.x * invs,
			rotationAxis.y * invs,
			rotationAxis.z * invs);
	}

	template<typename T>
	inline qua<T> conjugate(const qua<T>& q)
	{
		return qua<T>(q.w, -q.x, -q.y, -q.z);
	}

	template<typename T>
	inline qua<T> inverse(const qua<T>& q)
	{
		return conjugate(q) / dot(q, q);
	}
}