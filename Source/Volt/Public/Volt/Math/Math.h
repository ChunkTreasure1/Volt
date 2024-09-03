#pragma once

#include <CoreUtilities/Concepts.h>

#include <glm/glm.hpp>

namespace Math
{
	VT_INLINE glm::vec4 NormalizePlane(glm::vec4 p)
	{
		return p / glm::length(glm::vec3(p));
	}

	template<typename T = float>
	VT_INLINE glm::vec3 Scale(glm::vec3 const& v, T desiredLength)
	{
		return v * desiredLength / glm::length(v);
	}

	VT_INLINE static bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		using namespace glm;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<float>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<float>(0), epsilon<float>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
			LocalMatrix[3][3] = static_cast<float>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (uint32_t i = 0; i < 3; ++i)
			for (uint32_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = Scale(Row[0], static_cast<float>(1));
		scale.y = length(Row[1]);
		Row[1] = Scale(Row[1], static_cast<float>(1));
		scale.z = length(Row[2]);
		Row[2] = Scale(Row[2], static_cast<float>(1));

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0.f)
		{
			rotation.x = std::atan2(Row[1][2], Row[2][2]);
			rotation.z = std::atan2(Row[0][1], Row[0][0]);
		}
		else
		{
			rotation.x = std::atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

	VT_INLINE bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
	{
		using namespace glm;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<float>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<float>(0), epsilon<float>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<float>(0), epsilon<float>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<float>(0);
			LocalMatrix[3][3] = static_cast<float>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (uint32_t i = 0; i < 3; ++i)
			for (uint32_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = Scale(Row[0], static_cast<float>(1));
		scale.y = length(Row[1]);
		Row[1] = Scale(Row[1], static_cast<float>(1));
		scale.z = length(Row[2]);
		Row[2] = Scale(Row[2], static_cast<float>(1));

		int i, j, k = 0;
		float root, trace = Row[0].x + Row[1].y + Row[2].z;
		if (trace > static_cast<float>(0))
		{
			root = sqrt(trace + static_cast<float>(1.0));
			rotation.w = static_cast<float>(0.5) * root;
			root = static_cast<float>(0.5) / root;
			rotation.x = root * (Row[1].z - Row[2].y);
			rotation.y = root * (Row[2].x - Row[0].z);
			rotation.z = root * (Row[0].y - Row[1].x);
		} // End if > 0
		else
		{
			static int Next[3] = { 1, 2, 0 };
			i = 0;
			if (Row[1].y > Row[0].x) i = 1;
			if (Row[2].z > Row[i][i]) i = 2;
			j = Next[i];
			k = Next[j];

			root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<float>(1.0));

			rotation[i] = static_cast<float>(0.5) * root;
			root = static_cast<float>(0.5) / root;
			rotation[j] = root * (Row[i][j] + Row[j][i]);
			rotation[k] = root * (Row[i][k] + Row[k][i]);
			rotation.w = root * (Row[j][k] - Row[k][j]);
		} // End if <= 0

		return true;
	}

	template<typename T>
	VT_INLINE T DivideRoundUp(const T& numerator, const T& denominator)
	{
		return (numerator + denominator - T{ 1 }) / denominator;
	}

	template<Integer T>
	VT_INLINE T RoundToClosestMultiple(const T& number, const T& multiple)
	{
		T result = std::abs(number) + multiple / 2; 
		result -= result % multiple; 
		result *= number > 0 ? 1 : -1;

		return result;
	}

	template<Integer T>
	VT_INLINE T Get1DIndex(T x, T y, T z, T maxX, T maxY)
	{
		return (z + maxX * maxY) + (y * maxX) + x;
	}


}
