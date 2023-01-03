#pragma once

#include "Volt/Rendering/Camera/Frustum.h"

#include <gem/gem.h>

namespace Volt
{
	struct BoundingVolume
	{
		virtual const bool IsInFrusum(const Frustum& frustum, const gem::mat4& transform) const = 0;
		virtual const gem::vec3& GetCenter() const = 0;
		virtual const float GetRadius() const = 0;
	};

	struct BoundingSphere : public BoundingVolume
	{
		BoundingSphere() = default;
		BoundingSphere(const gem::vec3& center, float radius);

		const bool IsOnOrForwardPlane(const FrustumPlane& plane);
		const bool IsInFrusum(const Frustum& frustum, const gem::mat4& transform) const override;
		const gem::vec3& GetCenter() const override { return center; }
		const float GetRadius() const override { return radius; }


		gem::vec3 center = 0.f;
		float radius = 0.f;
	};

	struct BoundingBox : public BoundingVolume
	{
		BoundingBox() = default;
		BoundingBox(const gem::vec3& aMax, const gem::vec3& aMin);

		const bool IsOnOrForwardPlane(const FrustumPlane& plane);
		const bool IsInFrusum(const Frustum& frustum, const gem::mat4& transform) const override;
		const gem::vec3& GetCenter() const override { return max; }
		const float GetRadius() const override { return 0.f; }

		gem::vec3 max = 0.f;
		gem::vec3 min = 0.f;
	};
}