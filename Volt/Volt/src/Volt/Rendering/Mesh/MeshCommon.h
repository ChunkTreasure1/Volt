#pragma once

#include "Volt/Math/Math.h"

#include <glm/glm.hpp>

#include <xhash>

namespace Volt
{
	struct Meshlet
	{
		uint32_t vertexOffset;
		uint32_t triangleOffset;
		uint32_t vertexCount;
		uint32_t triangleCount;

		uint32_t objectId;
		uint32_t meshId;
		glm::uvec2 padding;

		glm::vec3 boundingSphereCenter;
		float boundingSphereRadius;

		glm::vec4 cone;
	};

	struct MeshletNew
	{
		glm::vec3 center;
		float radius;
		int8_t coneAxis[3];
		int8_t coneCutoff;

		uint32_t dataOffset;
		uint8_t vertexCount;
		uint8_t triangleCount;
	};

	struct Edge
	{
		uint32_t v0;
		uint32_t v1;

		inline bool operator==(const Edge& rhs) const
		{
			return v0 == rhs.v0 && v1 == rhs.v1;
		}
	};
}

namespace std
{
	template<typename T> struct hash;

	template<>
	struct hash<Volt::Edge>
	{
		std::size_t operator()(const Volt::Edge& edge) const
		{
			return Math::HashCombine(std::hash<uint32_t>()(edge.v0), std::hash<uint32_t>()(edge.v1));
		}
	};
}
