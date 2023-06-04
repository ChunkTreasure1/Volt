#pragma once
#include <GEM/gem.h>

namespace Volt
{
	inline bool rayTriangleIntersection(const gem::vec3& rayOrigin, const gem::vec3& rayDirection, const gem::vec3& v0, const gem::vec3& v1, const gem::vec3& v2, gem::vec3& intersectionPoint)
	{
		// Calculate the triangle normal
		gem::vec3 edge1 = v1 - v0;
		gem::vec3 edge2 = v2 - v0;
		gem::vec3 normal = gem::normalize(gem::cross(edge1, edge2));

		// Calculate the intersection point
		float t;
		gem::vec2 temp;
		bool intersects = gem::intersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, temp, t);
		intersectionPoint = rayOrigin + rayDirection * t;

		return intersects;
	}
}
