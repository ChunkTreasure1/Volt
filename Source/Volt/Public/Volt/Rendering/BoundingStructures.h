#pragma once

#include "Volt/Rendering/Camera/Frustum.h"

#include <glm/glm.hpp>

namespace Volt
{
	struct BoundingVolume
	{
		virtual const bool IsInFrusum(const Frustum& frustum, const glm::mat4& transform) const = 0;
		virtual const glm::vec3& GetCenter() const = 0;
		virtual const float GetRadius() const = 0;
	};

	struct BoundingSphere : public BoundingVolume
	{
		BoundingSphere() = default;
		BoundingSphere(const glm::vec3& center, float radius);

		const bool IsOnOrForwardPlane(const FrustumPlane& plane);
		const bool IsInFrusum(const Frustum& frustum, const glm::mat4& transform) const override;
		const glm::vec3& GetCenter() const override { return center; }
		const float GetRadius() const override { return radius; }

		glm::vec3 center = 0.f;
		float radius = 0.f;
	};

	struct BoundingBox : public BoundingVolume
	{
		BoundingBox() = default;
		BoundingBox(const glm::vec3& aMax, const glm::vec3& aMin);

		const bool IsOnOrForwardPlane(const FrustumPlane& plane);
		const bool IsInFrusum(const Frustum& frustum, const glm::mat4& transform) const override;
		const glm::vec3& GetCenter() const override { return max; }
		const float GetRadius() const override { return 0.f; }

		glm::vec3 max = 0.f;
		glm::vec3 min = 0.f;
	};
}
