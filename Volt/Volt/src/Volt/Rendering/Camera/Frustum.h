#pragma once

#include <GEM/gem.h>

namespace Volt
{
	struct FrustumPlane
	{
		FrustumPlane() = default;
		FrustumPlane(const gem::vec3& aPoint, const gem::vec3& aNormal)
			: point(aPoint), normal(aNormal)
		{
			normal = gem::normalize(normal);
		}

		const float GetSignedDistanceToPlane(const gem::vec3& position) const 
		{
			return gem::dot(position - point, normal);
		}

		gem::vec3 point = { 0.f, 0.f, 0.f };
		gem::vec3 normal = { 0.f, 1.f, 0.f };
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