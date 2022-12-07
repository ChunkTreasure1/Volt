#pragma once
#include "../constants.h"
#include "../epsilon.h"

namespace gem
{
	template<typename T = float>
	vec<3, T> scale(vec<3, T> const& v, T desiredLength)
	{
		return v * desiredLength / length(v);
	}

	template<typename T = float>
	bool decompose(const gem::mat4& transform, gem::vec3& translation, gem::vec3& rotation, gem::vec3& scale)
	{
		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (uint32_t i = 0; i < 3; ++i)
			for (uint32_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = gem::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = gem::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = gem::scale(Row[2], static_cast<T>(1));

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
}