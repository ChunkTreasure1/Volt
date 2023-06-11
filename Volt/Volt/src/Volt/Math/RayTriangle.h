#pragma once
#include <glm/glm.hpp>

#include <glm/gtx/intersect.hpp>

namespace Volt
{
	inline bool rayTriangleIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& intersectionPoint)
	{
		// Calculate the triangle normal
		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;
		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		// Calculate the intersection point
		float t;
		glm::vec2 temp;
		bool intersects = glm::intersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, temp, t);
		intersectionPoint = rayOrigin + rayDirection * t;

		return intersects;
	}
}
