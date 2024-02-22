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
		uint32_t padding;
		float clusterError = 0.f;
		
		float parentError = 0.f;
		glm::vec3 parentSphereCenter;

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
		explicit Edge(uint32_t iv0, uint32_t iv1)
		{
			if (iv0 > iv1)
			{
				std::swap(iv0, iv1);
			}

			v0 = iv0;
			v1 = iv1;

			hash = 0;
		}

		explicit Edge(uint32_t iv0, uint32_t iv1, const std::function<const glm::vec3&(uint32_t index)>& getPosition)
		{
			if (iv0 > iv1)
			{
				std::swap(iv0, iv1);
			}

			v0 = iv0;
			v1 = iv1;

			const auto& v0Pos = getPosition(v0);
			const auto& v1Pos = getPosition(v1);

			hash = Math::HashCombine(std::hash<uint32_t>()(v0), std::hash<uint32_t>()(v1));
			hash = Math::HashCombine(hash, std::hash<float>()(v0Pos.x));
			hash = Math::HashCombine(hash, std::hash<float>()(v0Pos.y));
			hash = Math::HashCombine(hash, std::hash<float>()(v0Pos.z));

			hash = Math::HashCombine(hash, std::hash<float>()(v1Pos.x));
			hash = Math::HashCombine(hash, std::hash<float>()(v1Pos.y));
			hash = Math::HashCombine(hash, std::hash<float>()(v1Pos.z));
		}

		uint32_t v0;
		uint32_t v1;

		size_t hash;

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
			return edge.hash != 0 ? edge.hash : Math::HashCombine(std::hash<uint32_t>()(edge.v0), std::hash<uint32_t>()(edge.v1));
		}
	};
}
