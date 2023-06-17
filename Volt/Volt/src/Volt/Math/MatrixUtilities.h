#pragma once

#include <glm/glm.hpp>

namespace Volt::Math
{
	inline glm::vec4 NormalizePlane(glm::vec4 p)
	{
		return p / glm::length(glm::vec3(p));
	}

	inline glm::mat4 Lerp(const glm::mat4& lhs, const glm::mat4& rhs, float factor)
	{
		glm::mat4 result = lhs;

		const glm::vec3 lhsRight = { lhs[0].x, lhs[0].y, lhs[0].z };
		const glm::vec3 rhsRight = { rhs[0].x, rhs[0].y, rhs[0].z };
		const glm::vec3 finalRight = nlerp(lhsRight, rhsRight, factor);

		result[0].x = finalRight.x;
		result[0].y = finalRight.y;
		result[0].z = finalRight.z;

		const glm::vec3 lhsUp = { lhs[1].x, lhs[1].y, lhs[1].z };
		const glm::vec3 rhsUp = { rhs[1].x, rhs[1].y, rhs[1].z };
		const glm::vec3 finalUp = nlerp(lhsUp, rhsUp, factor);

		result[1].x = finalUp.x;
		result[1].y = finalUp.y;
		result[1].z = finalUp.z;

		const glm::vec3 lhsFront = { lhs[2].x, lhs[2].y, lhs[2].z };
		const glm::vec3 rhsFront = { rhs[2].x, rhs[2].y, rhs[2].z };
		const glm::vec3 finalFront = nlerp(lhsFront, rhsFront, factor);

		result[2].x = finalFront.x;
		result[2].y = finalFront.y;
		result[2].z = finalFront.z;

		const glm::vec3 lhsPos = { lhs[3].x, lhs[3].y, lhs[3].z };
		const glm::vec3 rhsPos = { rhs[3].x, rhs[3].y, rhs[3].z };
		const glm::vec3 finalPos = lerp(lhsPos, rhsPos, factor);

		result[3].x = finalPos.x;
		result[3].y = finalPos.y;
		result[3].z = finalPos.z;

		return result;
	}
}