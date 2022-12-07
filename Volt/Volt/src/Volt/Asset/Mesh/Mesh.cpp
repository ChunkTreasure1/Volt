#include "vtpch.h"
#include "Mesh.h"

#include "Volt/Rendering/Buffer/VertexBuffer.h"
#include "Volt/Rendering/Buffer/IndexBuffer.h"

namespace Volt
{
	Mesh::Mesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material>& aMaterial)
	{
		myVertices = aVertices;
		myIndices = aIndices;

		myMaterial = aMaterial;

		SubMesh subMesh;
		subMesh.indexCount = (uint32_t)aIndices.size();
		subMesh.vertexCount = (uint32_t)aVertices.size();
		subMesh.vertexStartOffset = 0;
		subMesh.indexStartOffset = 0;

		mySubMeshes.push_back(subMesh);

		Construct();
	}

	void Mesh::Construct()
	{
		myVertexBuffer = VertexBuffer::Create(myVertices.data(), sizeof(Vertex) * (uint32_t)myVertices.size(), sizeof(Vertex));
		myIndexBuffer = IndexBuffer::Create(myIndices, (uint32_t)myIndices.size());

		myVertexBuffer->SetName(path.stem().string());

		gem::vec3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
		gem::vec3 max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (const auto& vertex : myVertices)
		{
			if (vertex.position < min)
			{
				min = vertex.position;
			}

			if (vertex.position > max)
			{
				max = vertex.position;
			}

			auto length = gem::distance(myBoundingSphere.center, vertex.position);
			if (length > myBoundingSphere.radius)
			{
				myBoundingSphere.radius = length;
			}
		}

		myBoundingBox = BoundingBox{ max, min };
	}
}