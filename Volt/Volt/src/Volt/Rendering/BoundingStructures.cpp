#include "vtpch.h"
#include "BoundingStructures.h"

namespace Volt
{
	BoundingSphere::BoundingSphere(const gem::vec3& aCenter, float aRadius)
		: center(aCenter), radius(aRadius)
	{
	}

	const bool BoundingSphere::IsOnOrForwardPlane(const FrustumPlane& plane)
	{
		float distance = plane.GetSignedDistanceToPlane(center);

		return distance > -radius;
	}
	const bool BoundingSphere::IsInFrusum(const Frustum& frustum, const gem::mat4& transform) const
	{
		const gem::vec3 globalScale = { gem::length(transform[0]), gem::length(transform[1]), gem::length(transform[2]) };
		const gem::vec3 globalCenter = transform * gem::vec4(center, 1.f);

		const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

		BoundingSphere tempSphere{ globalCenter, radius * maxScale };

		const bool l = tempSphere.IsOnOrForwardPlane(frustum.leftPlane);
		const bool r = tempSphere.IsOnOrForwardPlane(frustum.rightPlane);
		const bool f = tempSphere.IsOnOrForwardPlane(frustum.farPlane);
		const bool n = tempSphere.IsOnOrForwardPlane(frustum.nearPlane);
		const bool t = tempSphere.IsOnOrForwardPlane(frustum.topPlane);
		const bool b = tempSphere.IsOnOrForwardPlane(frustum.bottomPlane);

		return (l && r && f && n && t && b);
	}

	BoundingBox::BoundingBox(const gem::vec3& aMax, const gem::vec3& aMin)
		: max(aMax), min(aMin)
	{
	}

	const bool BoundingBox::IsOnOrForwardPlane(const FrustumPlane& plane)
	{
		return false;
	}
	const bool BoundingBox::IsInFrusum(const Frustum& frustum, const gem::mat4& transform) const
	{
		return false;
	}
}