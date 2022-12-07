#pragma once

#include <gem/gem.h>

namespace Volt::Math
{
	inline gem::vec4 NormalizePlane(gem::vec4 p)
	{
		return p / gem::length(gem::vec3(p));
	}

	inline gem::mat4 Lerp(const gem::mat4& lhs, const gem::mat4& rhs, float factor)
	{
		gem::mat4 result = lhs;

		const gem::vec3 lhsRight = { lhs[0].x, lhs[0].y, lhs[0].z };
		const gem::vec3 rhsRight = { rhs[0].x, rhs[0].y, rhs[0].z };
		const gem::vec3 finalRight = nlerp(lhsRight, rhsRight, factor);

		result[0].x = finalRight.x;
		result[0].y = finalRight.y;
		result[0].z = finalRight.z;

		const gem::vec3 lhsUp = { lhs[1].x, lhs[1].y, lhs[1].z };
		const gem::vec3 rhsUp = { rhs[1].x, rhs[1].y, rhs[1].z };
		const gem::vec3 finalUp = nlerp(lhsUp, rhsUp, factor);

		result[1].x = finalUp.x;
		result[1].y = finalUp.y;
		result[1].z = finalUp.z;

		const gem::vec3 lhsFront = { lhs[2].x, lhs[2].y, lhs[2].z };
		const gem::vec3 rhsFront = { rhs[2].x, rhs[2].y, rhs[2].z };
		const gem::vec3 finalFront = nlerp(lhsFront, rhsFront, factor);

		result[2].x = finalFront.x;
		result[2].y = finalFront.y;
		result[2].z = finalFront.z;

		const gem::vec3 lhsPos = { lhs[3].x, lhs[3].y, lhs[3].z };
		const gem::vec3 rhsPos = { rhs[3].x, rhs[3].y, rhs[3].z };
		const gem::vec3 finalPos = lerp(lhsPos, rhsPos, factor);

		result[3].x = finalPos.x;
		result[3].y = finalPos.y;
		result[3].z = finalPos.z;

		return result;
	}

	inline bool DecomposeTransform(const gem::mat4& transform, gem::vec3& trans, gem::vec3& rot, gem::vec3& scale)
	{
		using T = float;

		gem::mat4 localMatrix(transform);
		if (gem::epsilonEqual(localMatrix[3][3], static_cast<float>(0), gem::epsilon<T>()))
		{
			return false;
		}

		if (gem::epsilonNotEqual(localMatrix[0][3], static_cast<T>(0), gem::epsilon<T>()) ||
			gem::epsilonNotEqual(localMatrix[1][3], static_cast<T>(0), gem::epsilon<T>()) ||
			gem::epsilonNotEqual(localMatrix[2][3], static_cast<T>(0), gem::epsilon<T>()))
		{
			localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
			localMatrix[3][3] = static_cast<T>(1);
		}

		//Get the transform
		trans = gem::vec3(localMatrix[3]);
		localMatrix[3] = gem::vec4(0, 0, 0, localMatrix[3].w);

		gem::vec3 row[3];
		//Scale and shear
		for (uint32_t i = 0; i < 3; ++i)
		{
			for (uint32_t j = 0; j < 3; ++j)
			{
				row[i][j] = localMatrix[i][j];
			}
		}

		scale.x = length(row[0]);
		row[0] = gem::scale(row[0], static_cast<T>(1));
		scale.y = length(row[1]);
		row[1] = gem::scale(row[1], static_cast<T>(1));
		scale.z = length(row[2]);
		row[2] = gem::scale(row[2], static_cast<T>(1));

		rot.y = gem::asin(-row[0][2]);
		if (gem::cos(rot.y) != 0)
		{
			rot.x = std::atan2(row[1][2], row[2][2]);
			rot.z = std::atan2(row[0][1], row[0][0]);
		}
		else
		{
			rot.x = std::atan2(-row[2][0], row[1][1]);
			rot.z = 0;
		}

		return true;
	}
}