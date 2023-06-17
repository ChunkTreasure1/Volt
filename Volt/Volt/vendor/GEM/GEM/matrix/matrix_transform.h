#pragma once
#include "../vector/type_vector3.h"
#include "type_matrix4.h"

#ifdef GEM_FORCE_RIGHT_HANDED
#define GEM_RIGHT_HANDED
#else
#define GEM_LEFT_HANDED
#endif

namespace gem
{
	template<typename T>
	mat<4, 4, T> identity()
	{
		return mat<4, 4, T>(1.f);
	};

	template<typename T>
	mat<4, 4, T> lookAt(const vec<3, T>& eye, const vec<3, T>& center, const vec<3, T>& up)
	{
#ifdef GEM_LEFT_HANDED
		return lookAtLH(eye, center, up);
#else
		return lookAtRH(eye, center, up);
#endif
	};

	template<typename T>
	mat<4, 4, T> lookAtLH(const vec<3, T>& eye, const vec<3, T>& center, const vec<3, T>& up)
	{
		vec<3, T> const f(normalize(center - eye));
		vec<3, T> const s(normalize(cross(up, f)));
		vec<3, T> const u(cross(f, s));

		mat<4, 4, T> result(1);
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = f.x;
		result[1][2] = f.y;
		result[2][2] = f.z;
		result[3][0] = -dot(s, eye);
		result[3][1] = -dot(u, eye);
		result[3][2] = -dot(f, eye);
		return result;
	};

	template<typename T>
	mat<4, 4, T> lookAtRH(const vec<3, T>& eye, const vec<3, T>& center, const vec<3, T>& up)
	{
		vec<3, T> const f(normalize(center - eye));
		vec<3, T> const s(normalize(cross(f, up)));
		vec<3, T> const u(cross(s, f));

		mat<4, 4, T> result(1);
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = -f.x;
		result[1][2] = -f.y;
		result[2][2] = -f.z;
		result[3][0] = -dot(s, eye);
		result[3][1] = -dot(u, eye);
		result[3][2] = dot(f, eye);
		return result;
	};

	template<typename T>
	mat<4, 4, T> rotate(const mat<4, 4, T>& m, T angle, const vec<3, T>& v)
	{
		T const a = angle;
		T const c = cos(a);
		T const s = sin(a);

		vec<3, T> axis(normalize(v));
		vec<3, T> temp((T(1) - c) * axis);

		mat<4, 4, T> Rotate;
		Rotate[0][0] = c + temp[0] * axis[0];
		Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
		Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

		Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
		Rotate[1][1] = c + temp[1] * axis[1];
		Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

		Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
		Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
		Rotate[2][2] = c + temp[2] * axis[2];

		mat<4, 4, T> Result;
		Result[0] = m[0] * Rotate[0][0] + m[1] * Rotate[0][1] + m[2] * Rotate[0][2];
		Result[1] = m[0] * Rotate[1][0] + m[1] * Rotate[1][1] + m[2] * Rotate[1][2];
		Result[2] = m[0] * Rotate[2][0] + m[1] * Rotate[2][1] + m[2] * Rotate[2][2];
		Result[3] = m[3];
		return Result;
	};

	template<typename T>
	qua<T> rotate(qua<T> const& q, T const& angle, vec<3, T> const& v)
	{
		vec<3, T> Tmp = v;

		T len = gem::length(Tmp);
		if (abs(len - static_cast<T>(1)) > static_cast<T>(0.001))
		{
			T oneOverLen = static_cast<T>(1) / len;
			Tmp.x *= oneOverLen;
			Tmp.y *= oneOverLen;
			Tmp.z *= oneOverLen;
		}

		T const AngleRad(angle);
		T const Sin = sin(AngleRad * static_cast<T>(0.5));

		return q * qua<T>(cos(AngleRad * static_cast<T>(0.5)), Tmp.x * Sin, Tmp.y * Sin, Tmp.z * Sin);
	}

	template<typename T>
	vec<3, T> rotate(qua<T> const& q, vec<3, T> const& v)
	{
		return q * v;
	}

	template<typename T>
	mat<4, 4, T> scale(const mat<4, 4, T>& m, const vec<3, T>& v)
	{
		mat<4, 4, T> result;
		result[0] = m[0] * v[0];
		result[1] = m[1] * v[1];
		result[2] = m[2] * v[2];
		result[3] = m[3];
		return result;
	};

	template<typename T>
	mat<4, 4, T> translate(const mat<4, 4, T>& m, const vec<3, T>& v)
	{
		mat<4, 4, T> result(m);
		result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
		return result;
	};
}