#pragma once
#include "../qualifier.h"
#include <cassert>

namespace Pathfinder
{
	typedef vec<4, float> vec4;
	typedef vec<4, uint32_t> vec4ui;

	template<typename T>
	struct vec<4, T>
	{
	public:
		vec<4, T>() {};
		vec<4, T>(T val) : x(val), y(val), z(val), w(val) {};
		vec<4, T>(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {};
		vec<4, T>(const vec<4, T>& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {};
		vec<4, T>(const vec<3, T>& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(0) {};

		vec<4, T> operator+(const vec<4, T>& rhs) const { return vec<4, T>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); };
		vec<4, T> operator-(const vec<4, T>& rhs) const { return vec<4, T>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); };
		vec<4, T> operator*(const vec<4, T>& rhs) const { return vec<4, T>(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); };
		vec<4, T> operator/(const vec<4, T>& rhs) const { return vec<4, T>(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); };
		bool operator==(const vec<4, T>& rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w); };

		void operator+=(const vec<4, T>& rhs)
		{
			*this = *this + rhs;
		};
		void operator-=(const vec<4, T>& rhs)
		{
			*this = *this - rhs;
		};
		void operator*=(const vec<4, T>& rhs)
		{
			*this = *this * rhs;
		};
		void operator/=(const vec<4, T>& rhs)
		{
			*this = *this / rhs;
		};

		T& operator[](uint32_t index)
		{
			assert(index < 4 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		const T& operator[](uint32_t index) const
		{
			assert(index < 4 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		T x = 0, y = 0, z = 0, w = 0;
	};
}
