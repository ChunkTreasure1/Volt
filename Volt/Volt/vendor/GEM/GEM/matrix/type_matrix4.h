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
	typedef mat<4, 4, float> mat4;

	template<typename T>
	struct mat<4, 4, T>
	{
	public:
		mat<4, 4, T>() {};
		mat<4, 4, T>(T val)
		{
			data[0] = vec<4, T>(0);
			data[1] = vec<4, T>(0);
			data[2] = vec<4, T>(0);
			data[3] = vec<4, T>(0);

			data[0][0] = val;
			data[1][1] = val;
			data[2][2] = val;
			data[3][3] = val;
		};
		mat<4, 4, T>(vec<4, T> x, vec<4, T> y, vec<4, T> z, vec<4, T> w)
		{
			data[0] = x;
			data[1] = y;
			data[2] = z;
			data[3] = w;
		};
		mat<4, 4, T>(const mat<4, 4, T>& rhs)
		{
			data[0] = rhs.data[0];
			data[1] = rhs.data[1];
			data[2] = rhs.data[2];
			data[3] = rhs.data[3];
		};

		mat<4, 4, T>(const mat<3, 3, T>& rhs)
		{
			data[0] = vec<4, T>(rhs[0]);
			data[1] = vec<4, T>(rhs[1]);
			data[2] = vec<4, T>(rhs[2]);
			data[3] = { 0, 0, 0, 1 };
		};

		mat<4, 4, T> operator+(const mat<4, 4, T>& rhs) const
		{
			mat<4, 4, T> result;
			result[0] = data[0] + rhs.data[0];
			result[1] = data[1] + rhs.data[1];
			result[2] = data[2] + rhs.data[2];
			result[3] = data[3] + rhs.data[3];
			return result;
		};
		mat<4, 4, T> operator-(const mat<4, 4, T>& rhs) const
		{
			mat<4, 4, T> result;
			result[0] = data[0] - rhs.data[0];
			result[1] = data[1] - rhs.data[1];
			result[2] = data[2] - rhs.data[2];
			result[3] = data[3] - rhs.data[3];
			return result;
		};

		mat<4, 4, T> operator*(const mat<4, 4, T>& rhs) const
		{
			mat<4, 4, T> result(0.f);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
#ifdef GEM_MATRIX_ROWMAJOR
					result[i][j] = 0;
#endif
#ifdef GEM_MATRIX_COLUMNMAJOR
					result[j][i] = 0;
#endif
					for (int k = 0; k < 4; k++)
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
		bool operator==(const mat<4, 4, T>& rhs) const
		{
			return (
				data[0] == rhs.data[0] &&
				data[1] == rhs.data[1] &&
				data[2] == rhs.data[2] &&
				data[3] == rhs.data[3]
				);
		};

		void operator+=(const mat<4, 4, T>& rhs)
		{
			*this = *this + rhs;
		};
		void operator-=(const mat<4, 4, T>& rhs)
		{
			*this = *this - rhs;
		};
		void operator*=(const mat<4, 4, T>& rhs)
		{
			*this = *this * rhs;
		};

		vec<4, T>& operator[](uint32_t index)
		{
			assert(index < 4 && index >= 0);
			return data[index];
		};

		const vec<4, T>& operator[](uint32_t index) const
		{
			assert(index < 4 && index >= 0);
			return data[index];
		};

	private:
		std::array<vec<4, T>, 4> data;
	};

	template<typename T>
	vec<4, T> operator*(const mat<4, 4, T>& m, const vec<4, T>& v)
	{
		vec<4, T> Mov0(v[0]);
		vec<4, T> Mov1(v[1]);
		vec<4, T> Mul0 = m[0] * Mov0;
		vec<4, T> Mul1 = m[1] * Mov1;
		vec<4, T> Add0 = Mul0 + Mul1;
		vec<4, T> Mov2(v[2]);
		vec<4, T> Mov3(v[3]);
		vec<4, T> Mul2 = m[2] * Mov2;
		vec<4, T> Mul3 = m[3] * Mov3;
		vec<4, T> Add1 = Mul2 + Mul3;
		vec<4, T> Add2 = Add0 + Add1;

		return Add2;
	}
}
