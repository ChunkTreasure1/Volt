#pragma once
#include "../qualifier.h"
#include <cassert>

namespace gem
{
	typedef vec<3, float> vec3;
	typedef vec<3, uint32_t> vec3ui;

	template<typename T>
	struct vec<3, T>
	{
	public:
		vec<3, T>() {};
		vec<3, T>(T val) : x(val), y(val), z(val) {};
		vec<3, T>(T x, T y, T z) : x(x), y(y), z(z) {};
		vec<3, T>(const vec<3, T>& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {};
		vec<3, T>(const vec<4, T>& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {};

		vec<3, T> operator+(const vec<3, T>& rhs) const { return vec<3, T>(x + rhs.x, y + rhs.y, z + rhs.z); };
		vec<3, T> operator-(const vec<3, T>& rhs) const { return vec<3, T>(x - rhs.x, y - rhs.y, z - rhs.z); };
		vec<3, T> operator*(const vec<3, T>& rhs) const { return vec<3, T>(x * rhs.x, y * rhs.y, z * rhs.z); };
		vec<3, T> operator/(const vec<3, T>& rhs) const { return vec<3, T>(x / rhs.x, y / rhs.y, z / rhs.z); };
		
		bool operator<(const vec<3, T>& rhs) const { return (x < rhs.x && y < rhs.y && z < rhs.z); };
		bool operator>(const vec<3, T>& rhs) const { return (x > rhs.x && y > rhs.y && z > rhs.z); };
		bool operator==(const vec<3, T>& rhs) const { return (x == rhs.x && y == rhs.y && z == rhs.z); };
		bool operator!=(const vec<3, T>& rhs) const { return !(*this == rhs); };

		void operator+=(const vec<3, T>& rhs)
		{
			*this = *this + rhs;
		};
		void operator-=(const vec<3, T>& rhs)
		{
			*this = *this - rhs;
		};
		void operator*=(const vec<3, T>& rhs)
		{
			*this = *this * rhs;
		};
		void operator/=(const vec<3, T>& rhs)
		{
			*this = *this / rhs;
		};

		template<typename U>
		inline constexpr vec<3, T>& operator+=(U scalar)
		{
			this->x += static_cast<T>(scalar);
			this->y += static_cast<T>(scalar);
			this->z += static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator-=(U scalar)
		{
			this->x -= static_cast<T>(scalar);
			this->y -= static_cast<T>(scalar);
			this->z -= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator*=(U scalar)
		{
			this->x *= static_cast<T>(scalar);
			this->y *= static_cast<T>(scalar);
			this->z *= static_cast<T>(scalar);
			return *this;
		}

		template<typename U>
		inline constexpr vec<3, T>& operator/= (U scalar)
		{
			this->x /= static_cast<T>(scalar);
			this->y /= static_cast<T>(scalar);
			this->z /= static_cast<T>(scalar);
			return *this;
		}

		inline constexpr vec<3, T>& operator++()
		{
			++this->x;
			++this->y;
			++this->z;
			return *this;
		}

		inline constexpr vec<3, T>& operator--()
		{
			--this->x;
			--this->y;
			--this->z;
			return *this;
		}

		inline constexpr vec<3, T> operator++(int)
		{
			vec<3, T> Result(*this);
			++* this;
			return Result;
		}

		inline constexpr vec<3, T> operator--(int)
		{
			vec<3, T> Result(*this);
			--* this;
			return Result;
		}

		T& operator[](uint32_t index)
		{
			assert(index < 3 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		const T& operator[](uint32_t index) const
		{
			assert(index < 3 && index >= 0 && "index is out of range");
			return *(&x + index);
		};

		T x = 0, y = 0, z = 0;
	};

	template<typename T>
	inline constexpr vec<3, T> operator+(vec<3, T> const& v, T scalar)
	{
		return vec<3, T>(
			v.x + scalar,
			v.y + scalar,
			v.z + scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator+(T scalar, vec<3, T> const& v)
	{
		return vec<3, T>(
			v.x + scalar,
			v.y + scalar,
			v.z + scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator-(vec<3, T> const& v, T scalar)
	{
		return vec<3, T>(
			v.x - scalar,
			v.y - scalar,
			v.z - scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator-(T scalar, vec<3, T> const& v)
	{
		return vec<3, T>(
			scalar - v.x,
			scalar - v.y,
			scalar - v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator*(T scalar, vec<3, T> const& v)
	{
		return vec<3, T>(
			scalar * v.x,
			scalar * v.y,
			scalar * v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator*(vec<3, T> const& v, T scalar)
	{
		return vec<3, T>(
			scalar * v.x,
			scalar * v.y,
			scalar * v.z);
	}

	template<typename T>
	inline constexpr vec<3, T> operator/(vec<3, T> const& v, T scalar)
	{
		return vec<3, T>(
			v.x / scalar,
			v.y / scalar,
			v.z / scalar);
	}

	template<typename T>
	inline constexpr vec<3, T> operator/(T scalar, vec<3, T> const& v)
	{
		return vec<3, T>(
			scalar / v.x,
			scalar / v.y,
			scalar / v.z);
	}
}
