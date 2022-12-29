#include "vtpch.h"
#include "MeshCompiler.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Log/Log.h"

#include <fstream>

namespace Volt
{
	bool MeshCompiler::TryCompile(Ref<Mesh> mesh, const std::filesystem::path& destination, AssetHandle materialHandle)
	{
		if (!mesh || !mesh->IsValid())
		{
			VT_CORE_ERROR("Mesh is invalid or null!");
			return false;
		}

		if (materialHandle == Asset::Null())
		{
			CreateMaterial(mesh, destination);
		}

		/*
		* Encoding:
		* uint32_t: Sub mesh count
		* AssetHandle: Material handle
		*
		* uint32_t: Vertex count
		* vertices
		*
		* uint32_t: Index count
		* indices
		*
		* glm::vec3: Bounding sphere center
		* float: Bounding sphere radius
		*
		* Per sub mesh:
		* uint32_t: Material index
		* uint32_t: Vertex count
		* uint32_t: Index count
		* uint32_t: Vertex start offset
		* uint32_t: Index start offset
		*/

		std::vector<uint8_t> bytes;
		bytes.resize(CalculateMeshSize(mesh));

		size_t offset = 0;

		// Main
		{
			const uint32_t subMeshCount = (uint32_t)mesh->mySubMeshes.size();
			memcpy_s(&bytes[offset], sizeof(uint32_t), &subMeshCount, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			AssetHandle matHandle = materialHandle;
			if (matHandle == Asset::Null())
			{
				matHandle = mesh->myMaterial->handle;
			}

			memcpy_s(&bytes[offset], sizeof(AssetHandle), &matHandle, sizeof(AssetHandle));
			offset += sizeof(AssetHandle);

			const uint32_t vertexCount = (uint32_t)mesh->myVertices.size();
			memcpy_s(&bytes[offset], sizeof(uint32_t), &vertexCount, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy_s(&bytes[offset], sizeof(Vertex) * vertexCount, mesh->myVertices.data(), sizeof(Vertex) * vertexCount);
			offset += sizeof(Vertex) * vertexCount;

			const uint32_t indexCount = (uint32_t)mesh->myIndices.size();
			memcpy_s(&bytes[offset], sizeof(uint32_t), &indexCount, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy_s(&bytes[offset], sizeof(uint32_t) * indexCount, mesh->myIndices.data(), sizeof(uint32_t) * indexCount);
			offset += sizeof(uint32_t) * indexCount;

			const gem::vec3 boundingCenter = mesh->myBoundingSphere.center;
			memcpy_s(&bytes[offset], sizeof(gem::vec3), &boundingCenter, sizeof(gem::vec3));
			offset += sizeof(gem::vec3);

			const float boundingRadius = mesh->myBoundingSphere.radius;
			memcpy_s(&bytes[offset], sizeof(float), &boundingRadius, sizeof(float));
			offset += sizeof(float);
		}

		// Sub meshes
		{
			for (const auto& subMesh : mesh->mySubMeshes)
			{
				memcpy_s(&bytes[offset], sizeof(uint32_t), &subMesh.materialIndex, sizeof(uint32_t));
				offset += sizeof(uint32_t);

				memcpy_s(&bytes[offset], sizeof(uint32_t), &subMesh.vertexCount, sizeof(uint32_t));
				offset += sizeof(uint32_t);

				memcpy_s(&bytes[offset], sizeof(uint32_t), &subMesh.indexCount, sizeof(uint32_t));
				offset += sizeof(uint32_t);

				memcpy_s(&bytes[offset], sizeof(uint32_t), &subMesh.vertexStartOffset, sizeof(uint32_t));
				offset += sizeof(uint32_t);

				memcpy_s(&bytes[offset], sizeof(uint32_t), &subMesh.indexStartOffset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
			}
		}

		std::ofstream output(Volt::ProjectManager::GetPath() / destination, std::ios::binary);
		output.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
		output.close();

		return true;
	}

	size_t MeshCompiler::CalculateMeshSize(Ref<Mesh> mesh)
	{
		size_t size = 0;

		size += sizeof(uint32_t); // Sub mesh count
		size += sizeof(AssetHandle); // Material handle

		size += sizeof(uint32_t); // Vertex count
		size += sizeof(Vertex) * mesh->myVertices.size(); // Vertices
		size += sizeof(uint32_t); // Index count
		size += sizeof(uint32_t) * mesh->myIndices.size(); // Indices

		size += sizeof(gem::vec3); // Bounding sphere center
		size += sizeof(float); // Bounding sphere radius

		for (const auto& subMesh : mesh->mySubMeshes)
		{
			subMesh;

			size += sizeof(uint32_t); // Material index
			size += sizeof(uint32_t); // Vertex count
			size += sizeof(uint32_t); // Index count
			size += sizeof(uint32_t); // Vertex start offset
			size += sizeof(uint32_t); // Index start offset
		}

		return size;
	}

	void MeshCompiler::CreateMaterial(Ref<Mesh> mesh, const std::filesystem::path& destination)
	{
		mesh->myMaterial->path = destination.parent_path().string() + "\\" + mesh->path.stem().string() + ".vtmat";
		AssetManager::Get().SaveAsset(mesh->myMaterial);
	}
}