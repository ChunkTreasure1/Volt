#pragma once
#include "../qualifier.h"

#include <array>

namespace gem
{
	typedef qua<float> quat;

	template<typename T>
	struct qua
	{
	public:
		qua() {};
		qua(qua<T> const& q) : x(q.x), y(q.y), z(q.z), w(q.w) {};
		qua(T w, T x, T y, T z) : x(x), y(y), z(z), w(w) {};
		qua(T s, vec<3, T> const& v) : x(v.x), y(v.y), z(v.z), w(s) {};

		qua(vec<3, T> const& u, vec<3, T> const& v)
		{
			T norm_u_norm_v = sqrt(dot(u, u) * dot(v, v));
			T real_part = norm_u_norm_v + dot(u, v);
			vec<3, T> t;

			if (real_part < static_cast<T>(1.e-6f) * norm_u_norm_v)
			{
				real_part = static_cast<T>(0);
				t = abs(u.x) > abs(u.z) ? vec<3, T>(-u.y, u.x, static_cast<T>(0)) : vec<3, T>(static_cast<T>(0), -u.z, u.y);
			}
			else
			{
				t = cross(u, v);
			}

			*this = normalize(qua<T>(real_part, t.x, t.y, t.z));
		}

		qua(vec<3, T> const& eulerAngles)
		{
			vec<3, T> c = gem::cos(eulerAngles * T(0.5));
			vec<3, T> s = gem::sin(eulerAngles * T(0.5));

			this->w = c.x * c.y * c.z + s.x * s.y * s.z;
			this->x = s.x * c.y * c.z - c.x * s.y * s.z;
			this->y = c.x * s.y * c.z + s.x * c.y * s.z;
			this->z = c.x * c.y * s.z - s.x * s.y * c.z;
		}

		qua(mat<3, 3, T> const& m)
		{
			*this = quat_cast(m);
		}

		qua(mat<4, 4, T> const& m)
		{
			*this = quat_cast(m);
		}

		template<typename U>
		qua(qua<U> const& q) 
			: x(static_cast<T>(q.x)), y(static_cast<T>(q.y)), z(static_cast<T>(q.z)), w(static_cast<T>(q.w))
		{}
		
		template<typename T>
		qua<T>& operator+=(qua<T> const& p)
		{
			this->x += p.x;
			this->y += p.y;
			this->z += p.z;
			this->w += p.w;

			return *this;
		}

		int length() { return 4; }

		T& operator[](size_t i)
		{
			assert(i >= 0 && i < this->length() && "index out of range");
			return (&x)[i];
		}

		T const& operator[](size_t i) const
		{
			assert(i >= 0 && i < this->length() && "index out of range");
			return (&x)[i];
		}

		template<typename U>
		qua<T>& operator=(qua<U> const& q)
		{
			this->w = static_cast<T>(q.w);
			this->x = static_cast<T>(q.x);
			this->y = static_cast<T>(q.y);
			this->z = static_cast<T>(q.z);
			return *this;
		}

		template<typename U>
		qua<T>& operator*=(qua<U> const& r)
		{
			qua<T> const p(*this);
			qua<T> const q(r);

			this->w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
			this->x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
			this->y = p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z;
			this->z = p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x;
			return *this;
		}

		union
		{
			struct { T x, y, z, w; };
			std::array<T, 4> data;
		};
	};

	// -- Unary bit operators --

	template<typename T>
	qua<T> operator+(qua<T> const& q)
	{
		qua<T> result;
		result.x = (q.x > 0) ? q.x : -q.x;
		result.y = (q.y > 0) ? q.y : -q.y;
		result.z = (q.z > 0) ? q.z : -q.z;
		result.w = (q.w > 0) ? q.w : -q.w;
		return result;
	}

	template<typename T>
	qua<T> operator-(qua<T> const& q)
	{
		qua<T> result;
		result.x = (q.x > 0) ? -q.x : q.x;
		result.y = (q.y > 0) ? -q.y : q.y;
		result.z = (q.z > 0) ? -q.z : q.z;
		result.w = (q.w > 0) ? -q.w : q.w;
		return result;
	}

	// -- Binary operators --

	template<typename T>
	qua<T> operator+(qua<T> const& q, qua<T> const& p)
	{
		return qua<T>(q) += p;
	}

	template<typename T>
	qua<T> operator-(qua<T> const& q, qua<T> const& p)
	{
		return qua<T>(q) -= p;
	}

	template<typename T>
	qua<T> operator*(qua<T> const& q, qua<T> const& p)
	{
		return qua<T>(q) *= p;
	}

	template<typename T>
	vec<3, T> operator*(qua<T> const& q, vec<3, T> const& v)
	{
		vec<3, T> const QuatVector(q.x, q.y, q.z);
		vec<3, T> const uv(gem::cross(QuatVector, v));
		vec<3, T> const uuv(gem::cross(QuatVector, uv));

		return v + ((uv * q.w) + uuv) * static_cast<T>(2);
	}

	template<typename T>
	vec<3, T> operator*(vec<3, T> const& v, qua<T> const& q)
	{
		// tbc
		return vec<3, T>();
	}

	template<typename T>
	vec<4, T> operator*(qua<T> const& q, vec<4, T> const& v) 
	{
		// tbc
		return vec<4, T>();
	}

	template<typename T>
	vec<4, T> operator*(vec<4, T> const& v, qua<T> const& q)
	{
		// tbc
		return vec<4, T>();
	}

	template<typename T>
	qua<T> operator*(qua<T> const& q, T const& s)
	{
		return qua<T>(
			q.w * s, q.x * s, q.y * s, q.z * s);
	}

	template<typename T>
	qua<T> operator*(T const& s, qua<T> const& q)
	{
		return q * s;
	}

	template<typename T>
	qua<T> operator/(qua<T> const& q, T const& s)
	{
		return qua<T>(
			q.w / s, q.x / s, q.y / s, q.z / s);
	}

	// -- Boolean operators --

	template<typename T>
	bool operator==(qua<T> const& q1, qua<T> const& q2)
	{
		return q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w;
	}

	template<typename T>
	bool operator!=(qua<T> const& q1, qua<T> const& q2)
	{
		return q1.x != q2.x || q1.y != q2.y || q1.z != q2.z || q1.w != q2.w;
	}
}