#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Rendering/Buffer/CombinedVertexBuffer.h"
#include "Volt/Rendering/Buffer/CombinedIndexBuffer.h"

#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Buffer/VertexBuffer.h"
#include "Volt/Rendering/Buffer/IndexBuffer.h"

#include "Volt/Math/Math.h"

#include <meshoptimizer/meshoptimizer.h>

namespace Volt
{
	namespace Utility
	{
		static glm::vec2 OctNormalWrap(glm::vec2 v)
		{
			glm::vec2 wrap;
			wrap.x = (1.0f - glm::abs(v.y)) * (v.x >= 0.0f ? 1.0f : -1.0f);
			wrap.y = (1.0f - glm::abs(v.x)) * (v.y >= 0.0f ? 1.0f : -1.0f);
			return wrap;
		}

		static glm::vec2 OctNormalEncode(glm::vec3 n)
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

		// From https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/
		static float DiamondEncode(const glm::vec2& p)
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
		float EncodeTangent(const glm::vec3& normal, const glm::vec3& tangent)
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

	inline static BoundingSphere GetBoundingSphereFromVertices(const std::vector<Vertex>& vertices)
	{
		glm::vec3 minVertex(std::numeric_limits<float>::max());
		glm::vec3 maxVertex(-std::numeric_limits<float>::max());

		for (const auto& vertex : vertices)
		{
			minVertex = glm::min(minVertex, vertex.position);
			maxVertex = glm::max(maxVertex, vertex.position);
		}

		const glm::vec3 center = (minVertex + maxVertex) * 0.5f;

		float radius = 0.0f;
		for (const auto& vertex : vertices)
		{
			const float distanceSquared = glm::length2(vertex.position - center);
			if (distanceSquared > radius)
			{
				radius = distanceSquared;
			}
		}

		radius = std::sqrt(radius);

		return { center, radius };
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial)
	{
		myVertices = aVertices;
		myIndices = aIndices;

		myMaterial = aMaterial;

		SubMesh subMesh;
		subMesh.indexCount = (uint32_t)aIndices.size();
		subMesh.vertexCount = (uint32_t)aVertices.size();
		subMesh.vertexStartOffset = 0;
		subMesh.indexStartOffset = 0;
		subMesh.materialIndex = 0;

		subMesh.GenerateHash();

		mySubMeshes.push_back(subMesh);

		Construct();
	}

	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material> aMaterial, const std::vector<SubMesh>& subMeshes)
	{
		myVertices = aVertices;
		myIndices = aIndices;

		myMaterial = aMaterial;
		mySubMeshes = subMeshes;

		Construct();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Construct()
	{
		auto& bindlessData = Renderer::GetBindlessData();

		// Optimize mesh
		{
			meshopt_optimizeVertexFetch(myVertices.data(), myIndices.data(), myIndices.size(), myVertices.data(), myVertices.size(), sizeof(Vertex));
		}

		const auto encodedVertices = GetEncodedVertices();
		myVertexBuffer = VertexBuffer::Create(encodedVertices.data(), uint32_t(encodedVertices.size() * sizeof(EncodedVertex)));
		myIndexBuffer = IndexBuffer::Create(myIndices.data(), uint32_t(myIndices.size()));

		myIndexStartOffset = 0; //(uint32_t)indexStartLocation;
		myVertexStartOffset = 0; //(uint32_t)vertexStartLocation;

		for (auto& subMesh : mySubMeshes)
		{
			glm::vec3 t, r, s;
			Math::Decompose(subMesh.transform, t, r, s);

			myAverageScale += s;
		}

		myAverageScale /= (float)mySubMeshes.size();

		glm::vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		glm::vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (const auto& vertex : myVertices)
		{
			const glm::vec3 scaledPos = vertex.position * myAverageScale;

			if (glm::all(glm::lessThan(scaledPos, min)))
			{
				min = scaledPos;
			}

			if (glm::all(glm::lessThan(scaledPos, max)))
			{
				max = scaledPos * myAverageScale;
			}
		}

		myBoundingBox = BoundingBox{ max, min };
		myBoundingSphere = GetBoundingSphereFromVertices(myVertices);

		for (uint32_t i = 0; auto & subMesh : mySubMeshes)
		{
			std::vector<Vertex> subMeshVertices;
			subMeshVertices.insert(subMeshVertices.end(), std::next(myVertices.begin(), subMesh.vertexStartOffset), std::next(myVertices.begin(), subMesh.vertexStartOffset + subMesh.vertexCount));

			mySubMeshBoundingSpheres[i] = GetBoundingSphereFromVertices(subMeshVertices);
			i++;
		}
	}

	const std::vector<EncodedVertex> Mesh::GetEncodedVertices() const
	{
		std::vector<EncodedVertex> result{};
		result.reserve(myVertices.size());

		for (const auto& vertex : myVertices)
		{
			auto& encodedVertex = result.emplace_back();

			encodedVertex.position = vertex.position;

			// Encode normal
			{
				const auto octNormal = Utility::OctNormalEncode(vertex.normal);

				encodedVertex.normal.x = uint8_t(octNormal.x * 255);
				encodedVertex.normal.y = uint8_t(octNormal.y * 255);
			}

			// Encode tangent
			{
				encodedVertex.tangent = Utility::EncodeTangent(vertex.normal, vertex.tangent);
			}

			// Encode color
			{
				// #TODO_Ivar: Implement
			}

			// Tex coords
			{
				encodedVertex.texCoords[0] = static_cast<half_float::half>(vertex.texCoords.x);
				encodedVertex.texCoords[1] = static_cast<half_float::half>(vertex.texCoords.y);
			}

			// Influences
			{
				encodedVertex.influences.x = static_cast<uint16_t>(vertex.influences.x);
				encodedVertex.influences.y = static_cast<uint16_t>(vertex.influences.y);
				encodedVertex.influences.z = static_cast<uint16_t>(vertex.influences.z);
				encodedVertex.influences.w = static_cast<uint16_t>(vertex.influences.w);
			}

			// Weights
			{
				encodedVertex.weights[0] = static_cast<half_float::half>(vertex.weights.x);
				encodedVertex.weights[1] = static_cast<half_float::half>(vertex.weights.y);
				encodedVertex.weights[2] = static_cast<half_float::half>(vertex.weights.z);
				encodedVertex.weights[3] = static_cast<half_float::half>(vertex.weights.w);
			}
		}

		return result;
	}
}
