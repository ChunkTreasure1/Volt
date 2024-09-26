#pragma once

#include <glm/glm.hpp>

namespace Volt
{
	struct FrustumPlane
	{
		FrustumPlane() = default;
		FrustumPlane(const glm::vec3& aPoint, const glm::vec3& aNormal)
			: point(aPoint), normal(aNormal)
		{
			normal = glm::normalize(normal);
		}

		const float GetSignedDistanceToPlane(const glm::vec3& position) const
		{
			return glm::dot(position - point, normal);
		}

		glm::vec3 point = { 0.f, 0.f, 0.f };
		glm::vec3 normal = { 0.f, 1.f, 0.f };
	};

	struct Frustum
	{
		FrustumPlane topPlane;
		FrustumPlane bottomPlane;

		FrustumPlane rightPlane;
		FrustumPlane leftPlane;

		FrustumPlane farPlane;
		FrustumPlane nearPlane;
	};
}