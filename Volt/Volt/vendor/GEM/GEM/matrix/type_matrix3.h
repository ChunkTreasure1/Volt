#pragma once
#include "../qualifier.h"
#include "../vector/type_vector4.h"
#include <array>
#include <cassert>

#ifdef GEM_FORCE_ROWMAJOR
#define GEM_MATRIX_ROWMAJOR
#else
#define GEM_MATRIX_COLUMNMAJOR
#endif

namespace gem
{
	typedef mat<3, 3, float> mat3;

	template<typename T>
	struct mat<3, 3, T>
	{
	public:
		mat<3, 3, T>() {};
		mat<3, 3, T>(T val)
		{
			data[0] = vec<3, T>(0);
			data[1] = vec<3, T>(0);
			data[2] = vec<3, T>(0);

			data[0][0] = val;
			data[1][1] = val;
			data[2][2] = val;
		};
		mat<3, 3, T>(vec<3, T> x, vec<3, T> y, vec<3, T> z)
		{
			data[0] = x;
			data[1] = y;
			data[2] = z;
		};
		mat<3, 3, T>(const mat<3, 3, T>& rhs)
		{
			data[0] = rhs.data[0];
			data[1] = rhs.data[1];
			data[2] = rhs.data[2];
		};

		mat<3, 3, T>(const mat<4, 4, T>& rhs)
		{
			data[0] = { rhs[0][0], rhs[0][1], rhs[0][2] };
			data[1] = { rhs[1][0], rhs[1][1], rhs[1][2] };
			data[2] = { rhs[2][0], rhs[2][1], rhs[2][2] };
		};

		mat<3, 3, T> operator+(const mat<3, 3, T>& rhs) const
		{
			mat<3, 3, T> result;
			result[0] = data[0] + rhs.data[0];
			result[1] = data[1] + rhs.data[1];
			result[2] = data[2] + rhs.data[2];
			return result;
		};
		mat<3, 3, T> operator-(const mat<3, 3, T>& rhs) const
		{
			mat<3, 3, T> result;
			result[0] = data[0] - rhs.data[0];
			result[1] = data[1] - rhs.data[1];
			result[2] = data[2] - rhs.data[2];
			return result;
		};
		mat<3, 3, T> operator*(const mat<3, 3, T>& rhs) const
		{
			mat<3, 3, T> result(0.f);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
#ifdef GEM_MATRIX_ROWMAJOR
					result[i][j] = 0;
#endif
#ifdef GEM_MATRIX_COLUMNMAJOR
					result[j][i] = 0;
#endif
					for (int k = 0; k < 3; k++)
					{
#ifdef GEM_MATRIX_ROWMAJOR
						result[i][j] += data[i][k] * rhs.data[k][j];
#endif
#ifdef GEM_MATRIX_COLUMNMAJOR
						result[j][i] += data[k][i] * rhs.data[j][k];
#endif
					}
				}
			}
			return result;
		};

		vec<3, T> operator*(const vec<3, T>& v) const
		{
			return vec<3, T>(
				data[0][0] * v.x + data[1][0] * v.y + data[2][0] * v.z,
				data[0][1] * v.x + data[1][1] * v.y + data[2][1] * v.z,
				data[0][2] * v.x + data[1][2] * v.y + data[2][2] * v.z);
		}

		bool operator==(const mat<3, 3, T>& rhs) const
		{
			return (
				data[0] == rhs.data[0] &&
				data[1] == rhs.data[1] &&
				data[2] == rhs.data[2]
				);
		};

		void operator+=(const mat<3, 3, T>& rhs)
		{
			*this = *this + rhs;
		};
		void operator-=(const mat<3, 3, T>& rhs)
		{
			*this = *this - rhs;
		};
		void operator*=(const mat<3, 3, T>& rhs)
		{
			*this = *this * rhs;
		};

		vec<3, T>& operator[](uint32_t index)
		{
			assert(index < 3 && index >= 0);
			return data[index];
		};

		const vec<3, T>& operator[](uint32_t index) const
		{
			assert(index < 3 && index >= 0);
			return data[index];
		};

	private:
		std::array<vec<3, T>, 3> data;
	};
}
