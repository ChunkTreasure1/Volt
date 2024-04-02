#include "vtpch.h"
#include "Shape.h"

#include "Volt/Rendering/Vertex.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	Ref<Mesh> Shape::CreateUnitCube()
	{
		std::vector<Vertex> vertices =
		{
			{{  50,  50,  50 }},
			{{  50,  50, -50 }},
			{{  50, -50,  50 }},
			{{ -50,  50,  50 }},

			{{  50, -50, -50 }},
			{{ -50,  50, -50 }},
			{{ -50, -50,  50 }},
			{{ -50, -50, -50 }}

		};

		std::vector<uint32_t> indices =
		{
			//side right
			0,2,1,
			2,4,1,

			//side front
			1,4,7,
			7,5,1,

			//side left
			5,7,6,
			6,3,5,

			//side back
			3,6,0,
			6,2,0,

			//top
			3,0,5,
			0,1,5,

			//bottom
			6,4,2,
			6,7,4
		};

		//Ref<Material> material = AssetManager::GetAsset<Material>("Engine/Meshes/Primitives/SM_Cube.vtmat");
		Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices, AssetManager::CreateMemoryAsset<Material>("NewMat"));

		return mesh;
	}

	static void CalculateRing(size_t segments, float radius, float y, float dy, float height, float actualRadius, std::vector<Vertex>& vertices)
	{
		float segIncr = 1.0f / (float)(segments - 1);
		for (size_t s = 0; s < segments; s++)
		{
			float x = glm::cos(float(glm::pi<float>() * 2) * s * segIncr) * radius;
			float z = glm::sin(float(glm::pi<float>() * 2) * s * segIncr) * radius;

			Vertex& vertex = vertices.emplace_back();
			vertex.position = glm::vec3(actualRadius * x, actualRadius * y + height * dy, actualRadius * z);
		}
	}


	Ref<Mesh> Shape::CreateCapsule(float radius, float height)
	{
		constexpr size_t subdivisionsHeight = 8;
		constexpr size_t ringsBody = subdivisionsHeight + 1;
		constexpr size_t ringsTotal = subdivisionsHeight + ringsBody;
		constexpr size_t numSegments = 12;
		constexpr float radiusModifier = 0.021f; // Needed to ensure that the wireframe is always visible

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(numSegments * ringsTotal);
		indices.resize((numSegments - 1) * (ringsTotal - 1) * 2 * 3);

		float bodyIncr = 1.0f / (float)(ringsBody - 1);
		float ringIncr = 1.0f / (float)(subdivisionsHeight - 1);

		for (int r = 0; r < subdivisionsHeight / 2; r++)
		{
			CalculateRing(numSegments, glm::sin(glm::pi<float>() * r * ringIncr), glm::sin(glm::pi<float>() * (r * ringIncr - 0.5f)), -0.5f, height, radius + radiusModifier, vertices);
		}

		for (int r = 0; r < ringsBody; r++)
		{
			CalculateRing(numSegments, 1.0f, 0.0f, r * bodyIncr - 0.5f, height, radius + radiusModifier, vertices);
		}

		for (int r = subdivisionsHeight / 2; r < subdivisionsHeight; r++)
		{
			CalculateRing(numSegments, glm::sin(glm::pi<float>() * r * ringIncr), glm::sin(glm::pi<float>() * (r * ringIncr - 0.5f)), 0.5f, height, radius + radiusModifier, vertices);
		}

		uint32_t indexCount = 0;
		for (int r = 0; r < ringsTotal - 1; r++)
		{
			for (int s = 0; s < numSegments - 1; s++)
			{
				indices[indexCount] = (uint32_t)(r * numSegments + s + 1);
				indices[indexCount + 1] = (uint32_t)(r * numSegments + s + 1);
				indices[indexCount + 2] = (uint32_t)(r * numSegments + s + 1);

				indexCount += 3;

				indices[indexCount] = (uint32_t)((r + 1) * numSegments + s + 0);
				indices[indexCount + 1] = (uint32_t)((r + 1) * numSegments + s + 1);
				indices[indexCount + 2] = (uint32_t)(r * numSegments + s);
			
				indexCount += 3;
			}
		}

		Ref<Material> material = AssetManager::GetAsset<Material>("Engine/Meshes/Primitives/SM_Cube.vtmat");
		Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices, material);

		return mesh;
	}
}
