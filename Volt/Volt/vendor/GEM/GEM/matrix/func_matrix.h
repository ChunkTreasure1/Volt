#pragma once
#include "type_matrix3.h"
#include "type_matrix4.h"

namespace gem
{
	template<typename T>
	static mat<3, 3, T> transpose(mat<3, 3, T> const& m)
	{
		mat<3, 3, T> result;
		result[0][0] = m[0][0];
		result[0][1] = m[1][0];
		result[0][2] = m[2][0];

		result[1][0] = m[0][1];
		result[1][1] = m[1][1];
		result[1][2] = m[2][1];

		result[2][0] = m[0][2];
		result[2][1] = m[1][2];
		result[2][2] = m[2][2];
		return result;
	}

	template<typename T>
	static mat<4, 4, T> transpose(mat<4, 4, T> const& m)
	{
		mat<4, 4, T> result;
		result[0][0] = m[0][0];
		result[0][1] = m[1][0];
		result[0][2] = m[2][0];
		result[0][3] = m[3][0];

		result[1][0] = m[0][1];
		result[1][1] = m[1][1];
		result[1][2] = m[2][1];
		result[1][3] = m[3][1];

		result[2][0] = m[0][2];
		result[2][1] = m[1][2];
		result[2][2] = m[2][2];
		result[2][3] = m[3][2];

		result[3][0] = m[0][3];
		result[3][1] = m[1][3];
		result[3][2] = m[2][3];
		result[3][3] = m[3][3];
		return result;
	}

	template<typename T>
	static mat<3, 3, T> inverse(mat<3, 3, T> const& m)
	{
		T OneOverDeterminant = static_cast<T>(1) / (
			+m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
			- m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
			+ m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

		mat<3, 3, T> inverse;
		inverse[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * OneOverDeterminant;
		inverse[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * OneOverDeterminant;
		inverse[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * OneOverDeterminant;
		inverse[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * OneOverDeterminant;
		inverse[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * OneOverDeterminant;
		inverse[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * OneOverDeterminant;
		inverse[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * OneOverDeterminant;
		inverse[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * OneOverDeterminant;
		inverse[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * OneOverDeterminant;

		return inverse;
	}

	template<typename T>
	static mat<4, 4, T> inverse(mat<4, 4, T> const& m)
	{
		T Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		T Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		T Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		T Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		T Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		T Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		T Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		T Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		T Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		T Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		T Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		T Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		T Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		T Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		T Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		T Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		T Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		T Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		vec<4, T> Fac0(Coef00, Coef00, Coef02, Coef03);
		vec<4, T> Fac1(Coef04, Coef04, Coef06, Coef07);
		vec<4, T> Fac2(Coef08, Coef08, Coef10, Coef11);
		vec<4, T> Fac3(Coef12, Coef12, Coef14, Coef15);
		vec<4, T> Fac4(Coef16, Coef16, Coef18, Coef19);
		vec<4, T> Fac5(Coef20, Coef20, Coef22, Coef23);

		vec<4, T> Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
		vec<4, T> Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
		vec<4, T> Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
		vec<4, T> Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);

		vec<4, T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
		vec<4, T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
		vec<4, T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
		vec<4, T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

		vec<4, T> SignA(+1, -1, +1, -1);
		vec<4, T> SignB(-1, +1, -1, +1);
		mat<4, 4, T> inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

		vec<4, T> Row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

		vec<4, T> Dot0(m[0] * Row0);
		T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

		T OneOverDeterminant = static_cast<T>(1) / Dot1;

		return inverse * OneOverDeterminant;
	}
}