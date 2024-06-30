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

	struct VertexMaterialData
	{
		glm::vec<4, uint8_t> normal;
		float tangent = 0.f;
		glm::vec<2, half_float::half> texCoords = glm::vec<2, half_float::half>(0.f, 0.f);
	};

	struct VertexAnimationInfo
	{
		uint16_t influenceCount;
		uint16_t boneOffset;
	};

	struct VertexAnimationData
	{
		glm::uvec4 influences = 0u;
		glm::vec4 weights = 0.f;
	};

	namespace Utility
	{
		inline glm::vec2 OctNormalWrap(glm::vec2 v)
		{
			glm::vec2 wrap;
			wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
			wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
			return wrap;
		}

		inline glm::vec2 OctNormalEncode(glm::vec3 n)
		{
			n /= (glm::abs(n.x) + glm::abs(n.y) + glm::abs(n.z));

			glm::vec2 wrapped = OctNormalWrap(n);

			glm::vec2 result;
			result.x = n.z >= 0.0f ? n.x : wrapped.x;
			result.y = n.z >= 0.0f ? n.y : wrapped.y;

			result.x = result.x * 0.5f + 0.5f;
			result.y = result.y * 0.5f + 0.5f;

			return result;
		}

		inline glm::vec3 OctNormalDecode(glm::vec2 f)
		{
			f = f * 2.f - 1.f;

			// https://twitter.com/Stubbesaurus/status/937994790553227264
			glm::vec3 n = glm::vec3(f.x, f.y, 1.f - abs(f.x) - abs(f.y));
			float t = glm::clamp(-n.z, 0.f, 1.f);

			n.x += n.x >= 0.0f ? -t : t;
			n.y += n.y >= 0.0f ? -t : t;

			return normalize(n);
		}

		// From https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/
		inline float DiamondEncode(const glm::vec2& p)
		{
			// Project to the unit diamond, then to the x-axis.
			float x = p.x / (glm::abs(p.x) + glm::abs(p.y));

			// Contract the x coordinate by a factor of 4 to represent all 4 quadrants in
			// the unit range and remap
			float pySign = 0.f;
			if (p.y < 0.f)
			{
				pySign = -1.f;
			}
			else if (p.y > 0.f)
			{
				pySign = 1.f;
			}

			return -pySign * 0.25f * x + 0.5f + pySign * 0.25f;
		}

		// Given a normal and tangent vector, encode the tangent as a single float that can be
		// subsequently quantized.
		inline float EncodeTangent(const glm::vec3& normal, const glm::vec3& tangent)
		{
			// First, find a canonical direction in the tangent plane
			glm::vec3 t1;
			if (abs(normal.y) > abs(normal.z))
			{
				// Pick a canonical direction orthogonal to n with z = 0
				t1 = glm::vec3(normal.y, -normal.x, 0.f);
			}
			else
			{
				// Pick a canonical direction orthogonal to n with y = 0
				t1 = glm::vec3(normal.z, 0.f, -normal.x);
			}
			t1 = normalize(t1);

			// Construct t2 such that t1 and t2 span the plane
			glm::vec3 t2 = cross(t1, normal);

			// Decompose the tangent into two coordinates in the canonical basis
			glm::vec2 packed_tangent = glm::vec2(dot(tangent, t1), dot(tangent, t2));

			// Apply our diamond encoding to our two coordinates
			return DiamondEncode(packed_tangent);
		}
	}
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
