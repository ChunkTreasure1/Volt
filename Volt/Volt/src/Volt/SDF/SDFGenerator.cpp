#include "vtpch.h"
#include "SDFGenerator.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include <CoreUtilities/Containers/Vector.h>

namespace Volt
{
	SDFGenerator::SDFGenerator()
	{
	}

	Vector<MeshSDF> SDFGenerator::Generate(Mesh& mesh)
	{
		Vector<MeshSDF> result(mesh.GetSubMeshes().size());

		for (uint32_t i = 0; const auto & subMesh : mesh.GetSubMeshes())
		{
			result[i] = GenerateForSubMesh(mesh, i, subMesh);
			i++;
		}

		return result;
	}

	constexpr float RESOLUTION = 10.f; // One point equals 10cm
	constexpr float TRIANGLE_THICKNESS = 1.f; // 1 cm

	inline float Dot2(const glm::vec3& v) { return glm::dot(v, v); }

	// From https://iquilezles.org/articles/triangledistance/
	inline float UDFTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& p)
	{
		glm::vec3 v21 = v2 - v1; glm::vec3 p1 = p - v1;
		glm::vec3 v32 = v3 - v2; glm::vec3 p2 = p - v2;
		glm::vec3 v13 = v1 - v3; glm::vec3 p3 = p - v3;

		glm::vec3 nor = glm::cross(v21, v13);

		return glm::sqrt(
			// Inside/outside test
			(glm::sign(glm::dot(glm::cross(v21, nor), p1)) +
			 glm::sign(glm::dot(glm::cross(v32, nor), p2)) +
			 glm::sign(glm::dot(glm::cross(v13, nor), p3)) < 2.f)
			?
			// 3 edges
			glm::min(glm::min(
			Dot2(v21 * glm::clamp(glm::dot(v21, p1) / Dot2(v21), 0.f, 1.f) - p1),
			Dot2(v32 * glm::clamp(glm::dot(v32, p2) / Dot2(v32), 0.f, 1.f) - p2)),
			Dot2(v13 * glm::clamp(glm::dot(v13, p3) / Dot2(v13), 0.f, 1.f) - p3))
			:
			//1 face
			glm::dot(nor, p1) * glm::dot(nor, p1) / Dot2(nor));
	}

	inline uint32_t Get1DIndex(uint32_t x, uint32_t y, uint32_t z, uint32_t maxX, uint32_t maxY)
	{
		return (z * maxX * maxY) + (y * maxX) + x;
	}

	MeshSDF SDFGenerator::GenerateForSubMesh(Mesh& mesh, const uint32_t subMeshIndex, const SubMesh& subMesh)
	{
		const auto boundingBox = mesh.GetSubMeshBoundingBox(subMeshIndex);

		const glm::vec3 localMin = boundingBox.min;
		const glm::vec3 localMax = boundingBox.max;

		const float width = glm::max(glm::abs(localMax.x - localMin.x), TRIANGLE_THICKNESS);
		const float height = glm::max(glm::abs(localMax.y - localMin.y), TRIANGLE_THICKNESS);
		const float depth = glm::max(glm::abs(localMax.z - localMin.z), TRIANGLE_THICKNESS);

		const uint32_t pointCountWidth = static_cast<uint32_t>(glm::ceil(width / RESOLUTION));
		const uint32_t pointCountHeight = static_cast<uint32_t>(glm::ceil(height / RESOLUTION));
		const uint32_t pointCountDepth = static_cast<uint32_t>(glm::ceil(depth / RESOLUTION));

		Vector<float> sdfMap(pointCountWidth * pointCountHeight * pointCountDepth, 100'000.f);

		const auto& meshIndices = mesh.GetIndices();
		const auto& vertexPositions = mesh.GetVertexContainer().positions;

		for (uint32_t z = 0; z < pointCountDepth; ++z)
		{
			for (uint32_t y = 0; y < pointCountHeight; ++y)
			{
				for (uint32_t x = 0; x < pointCountWidth; ++x)
				{
					const glm::vec3 pointPos = glm::vec3{ x * RESOLUTION, y * RESOLUTION, z * RESOLUTION } + localMin;

					for (uint32_t idx = subMesh.indexStartOffset; idx < subMesh.indexStartOffset + subMesh.indexCount; idx += 3)
					{
						const uint32_t idx0 = meshIndices.at(idx + 0) + subMesh.vertexStartOffset;
						const uint32_t idx1 = meshIndices.at(idx + 1) + subMesh.vertexStartOffset;
						const uint32_t idx2 = meshIndices.at(idx + 2) + subMesh.vertexStartOffset;
						
						const glm::vec3& v0 = vertexPositions.at(idx0);
						const glm::vec3& v1 = vertexPositions.at(idx1);
						const glm::vec3& v2 = vertexPositions.at(idx2);
					
						const uint32_t index = Get1DIndex(x, y, z, pointCountWidth, pointCountHeight);
						sdfMap[index] = glm::min(UDFTriangle(v0, v1, v2, pointPos) - TRIANGLE_THICKNESS, sdfMap[index]);
					}
				}
			}
		}

		MeshSDF result;
		result.sdf = sdfMap;
		result.size = { pointCountWidth, pointCountHeight, pointCountDepth };

		return result;
	}
}
