#pragma once
#include "../qualifier.h"
#include <cassert>

namespace gem
{
	typedef vec<2, float> vec2;
	typedef vec<2, uint32_t> vec2ui;

	template<typename T>
	struct vec<2, T>
	{
	public:
		vec<2, T>() {};
		vec<2, T>(T val) : x(val), y(val) {};
		vec<2, T>(T x, T y) : x(x), y(y) {};
		vec<2, T>(const vec<2, T>& rhs) : x(rhs.x), y(rhs.y) {};
		vec<2, T>(const vec<3, T>& rhs) : x(rhs.x), y(rhs.y) {};
		vec<2, T>(const vec<4, T>& rhs) : x(rhs.x), y(rhs.y) {};

		vec<2, T> operator+(const vec<2, T>& rhs) const { return vec<2, T>(x + rhs.x, y + rhs.y); };
		vec<2, T> operator-(const vec<2, T>& rhs) const { return vec<2, T>(x - rhs.x, y - rhs.y); };
		vec<2, T> operator*(const vec<2, T>& rhs) const { return vec<2, T>(x * rhs.x, y * rhs.y); };
		vec<2, T> operator/(const vec<2, T>& rhs) const { return vec<2, T>(x / rhs.x, y / rhs.y); };
		bool operator==(const vec<2, T>& rhs) const { return (x == rhs.x && y == rhs.y); };

		void operator+=(const vec<2, T>& rhs)
		{
			*this = *this + rhs;
		};
		void operator-=(const vec<2, T>& rhs)
		{
			*this = *this - rhs;
		};
		void operator*=(const vec<2, T>& rhs)
		{
			*this = *this * rhs;
		};
		void operator/=(const vec<2, T>& rhs)
		{
			*this = *this / rhs;
		};

		T& operator[](uint32_t index)
		{
			assert(index < 2 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		const T& operator[](uint32_t index) const
		{
			assert(index < 2 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		T x, y = 0;
	};
}
