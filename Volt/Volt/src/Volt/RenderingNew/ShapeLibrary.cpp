#include "vtpch.h"
#include "ShapeLibrary.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Asset/Rendering/Material.h"

namespace Volt
{
	struct MeshData
	{
		AssetHandle cubeHandle = Asset::Null();
		AssetHandle sphereHandle = Asset::Null();
	};

	static MeshData s_meshData;


	static AssetHandle CreateCube()
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.resize(24);
		indices.resize(36);

		constexpr float SIDE = 0.5f;

		const glm::vec3 positions[8] =
		{
			{ -SIDE, SIDE, -SIDE },
			{ SIDE, SIDE, -SIDE },
			{ SIDE, -SIDE, -SIDE },
			{ -SIDE, -SIDE, -SIDE },
			{ -SIDE, SIDE, SIDE },
			{ SIDE, SIDE, SIDE },
			{ SIDE, -SIDE, SIDE },
			{ -SIDE, -SIDE, SIDE }
		};

		const glm::vec3 normals[6] =
		{
			{ 0.f, 0.f, -1.f },
			{ 0.f, 0.f, 1.f },
			{ 0.f, 1.f, 0.f },
			{ 0.f, -1.f, 0.f },
			{ -1.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f }
		};

		const glm::vec3 tangents[6] =
		{
			{ 1.f, 0.f, 0.f },
			{ -1.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f },
			{ 0.f, 0.f, -1.f },
			{ 0.f, 0.f, 1.f }
		};

		const glm::vec2 uvs[4] =
		{
			{ 0.f, 1.f },
			{ 1.f, 1.f },
			{ 1.f, 0.f },
			{ 0.f, 0.f }
		};

		for (size_t i = 0; i < 6; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				const size_t vertexIndex = i * 4 + j;
				vertices[vertexIndex].position = positions[i * 2 + (j % 2)];
				vertices[vertexIndex].normal = normals[i];
				vertices[vertexIndex].tangent = tangents[i];
				vertices[vertexIndex].texCoords = uvs[j];
			}
		}

		for (uint32_t i = 0; i < 6; i++)
		{
			const uint32_t baseIndex = i * 6;
			indices[baseIndex + 0] = i * 4;
			indices[baseIndex + 1] = i * 4 + 1;
			indices[baseIndex + 2] = i * 4 + 2;
			indices[baseIndex + 3] = i * 4;
			indices[baseIndex + 4] = i * 4 + 2;
			indices[baseIndex + 5] = i * 4 + 3;
		}

		Ref<Material> material = AssetManager::CreateMemoryAsset<Material>("Default");
		material->Compile();

		Ref<Mesh> mesh = AssetManager::CreateMemoryAsset<Mesh>("Cube", vertices, indices, material);
		return mesh->handle;
	}

	static AssetHandle CreateSphere()
	{
		struct Triangle
		{
			uint32_t v1, v2, v3;

			Triangle(uint32_t v1, uint32_t v2, uint32_t v3)
				: v1(v1), v2(v2), v3(v3)
			{
			}
		};
		
		auto addVertex = [](std::vector<Vertex>& vertices, const glm::vec3& position, const glm::vec2& texCoords) -> uint32_t
		{
			auto& vertex = vertices.emplace_back();
			vertex.position = position;
			vertex.texCoords = texCoords;

			return static_cast<uint32_t>(vertices.size()) - 1;
		};

		auto subdivide = [&addVertex](auto subdivide, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const uint32_t& v1, const uint32_t& v2, const uint32_t& v3, int32_t depth)
		{
			if (depth == 0)
			{
				triangles.emplace_back(v1, v2, v3);
				return;
			}

			const uint32_t middle1 = addVertex(vertices, glm::normalize(vertices[v1].position + vertices[v2].position), glm::vec2(0.0f, 0.0f));
			const uint32_t middle2 = addVertex(vertices, glm::normalize(vertices[v2].position + vertices[v3].position), glm::vec2(0.5f, 0.0f));
			const uint32_t middle3 = addVertex(vertices, glm::normalize(vertices[v3].position + vertices[v1].position), glm::vec2(1.0f, 0.0f));

			subdivide(subdivide, vertices, triangles, v1, middle1, middle3, depth - 1);
			subdivide(subdivide, vertices, triangles, middle1, v2, middle2, depth - 1);
			subdivide(subdivide, vertices, triangles, middle3, middle2, v3, depth - 1);
			subdivide(subdivide, vertices, triangles, middle1, middle2, middle3, depth - 1);
		};

		std::vector<Vertex> vertices;
		std::vector<Triangle> triangles;

		const float t = (1.f + std::sqrt(5.f)) / 2.f;

		const glm::vec3 icosahedronVertices[12] =
		{
			glm::normalize(glm::vec3(-1, t, 0)),
			glm::normalize(glm::vec3(1, t, 0)),
			glm::normalize(glm::vec3(-1, -t, 0)),
			glm::normalize(glm::vec3(1, -t, 0)),

			glm::normalize(glm::vec3(0, -1, t)),
			glm::normalize(glm::vec3(0, 1, t)),
			glm::normalize(glm::vec3(0, -1, -t)),
			glm::normalize(glm::vec3(0, 1, -t)),

			glm::normalize(glm::vec3(t, 0, -1)),
			glm::normalize(glm::vec3(t, 0, 1)),
			glm::normalize(glm::vec3(-t, 0, -1)),
			glm::normalize(glm::vec3(-t, 0, 1))
		};

		Triangle icosahedronIndices[20] = 
		{
			{0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
			{1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},

			{3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
			{4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
		};

		constexpr int32_t SUBDIVISIONS = 3;

		// Add vertices of the icosahedron
		for (const auto& vertex : icosahedronVertices)
		{
			Vertex v;
			v.position = vertex;
			vertices.push_back(v);
		}

		// Subdivide each face of the icosahedron
		for (const auto& triangle : icosahedronIndices)
		{
			subdivide(subdivide, vertices, triangles, triangle.v1, triangle.v2, triangle.v3, SUBDIVISIONS);
		}

		// Calculate normals, tangents, and update UVs
		for (auto& vertex : vertices)
		{
			vertex.normal = glm::normalize(vertex.position);
			vertex.tangent = glm::normalize(glm::cross(vertex.normal, glm::vec3(0.0f, 1.0f, 0.0f)));

			// Calculate UVs using spherical coordinates
			float theta = std::atan2(vertex.position.z, vertex.position.x) + glm::pi<float>();
			float phi = std::acos(vertex.position.y);
			vertex.texCoords = glm::vec2(theta / (2.0f * glm::pi<float>()), phi / glm::pi<float>());
		}

		std::vector<uint32_t> indices;
		for (const auto& tri : triangles)
		{
			indices.emplace_back(tri.v1);
			indices.emplace_back(tri.v2);
			indices.emplace_back(tri.v3);
		}

		Ref<Material> material = AssetManager::CreateMemoryAsset<Material>("Default");
		material->Compile();

		Ref<Mesh> mesh = AssetManager::CreateMemoryAsset<Mesh>("Icosphere", vertices, indices, material);
		return mesh->handle;
	}

	Ref<Mesh> ShapeLibrary::GetCube()
	{
		if (s_meshData.cubeHandle == Asset::Null())
		{
			s_meshData.cubeHandle = CreateCube();
		}

		return AssetManager::GetAsset<Mesh>(s_meshData.cubeHandle);
	}

	Ref<Mesh> ShapeLibrary::GetSphere()
	{
		if (s_meshData.sphereHandle == Asset::Null())
		{
			s_meshData.sphereHandle = CreateSphere();
		}

		return AssetManager::GetAsset<Mesh>(s_meshData.sphereHandle);
	}
}
