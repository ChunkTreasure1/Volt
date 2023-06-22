#include "vtpch.h"
#include "MeshCompiler.h"

#include "Volt/Core/BinarySerializer.h"

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

		AssetHandle matHandle = materialHandle;
		if (matHandle == Asset::Null())
		{
			matHandle = mesh->myMaterial->handle;
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
		* uint32_t: nameCount
		*
		* Per name:
		* uint32_t: nameSize
		* uint8_t: stringBytes
		*
		* Per sub mesh:
		* uint32_t: Material index
		* uint32_t: Vertex count
		* uint32_t: Index count
		* uint32_t: Vertex start offset
		* uint32_t: Index start offset
		* glm::mat4: transform
		*/

		BinarySerializer serializer{ Volt::ProjectManager::GetDirectory() / destination, CalculateMeshSize(mesh) };

		const uint32_t submeshCount = (uint32_t)mesh->mySubMeshes.size();
		const uint32_t vertexCount = (uint32_t)mesh->myVertices.size();
		const uint32_t indexCount = (uint32_t)mesh->myIndices.size();

		const glm::vec3 boundingCenter = mesh->myBoundingSphere.center;
		const float boundingRadius = mesh->myBoundingSphere.radius;

		serializer.Serialize<uint32_t>(submeshCount);
		serializer.Serialize<AssetHandle>(matHandle);

		serializer.Serialize<uint32_t>(vertexCount);
		serializer.Serialize(mesh->myVertices.data(), sizeof(Vertex) * vertexCount);

		serializer.Serialize<uint32_t>(indexCount);
		serializer.Serialize(mesh->myIndices.data(), sizeof(uint32_t) * indexCount);

		serializer.Serialize<glm::vec3>(boundingCenter);
		serializer.Serialize<float>(boundingRadius);

		serializer.Serialize<uint32_t>(submeshCount);

		for (const auto& subMesh : mesh->mySubMeshes)
		{
			serializer.Serialize<uint32_t>((uint32_t)subMesh.name.size());
			serializer.Serialize<std::string>(subMesh.name);
		}

		for (const auto& subMesh : mesh->mySubMeshes)
		{
			serializer.Serialize<uint32_t>(subMesh.materialIndex);
			serializer.Serialize<uint32_t>(subMesh.vertexCount);
			serializer.Serialize<uint32_t>(subMesh.indexCount);
			serializer.Serialize<uint32_t>(subMesh.vertexStartOffset);
			serializer.Serialize<uint32_t>(subMesh.indexStartOffset);
			serializer.Serialize<glm::mat4>(subMesh.transform);
		}

		serializer.WriteToFile();
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

		size += sizeof(glm::vec3); // Bounding sphere center
		size += sizeof(float); // Bounding sphere radius

		// Sub mesh names
		size += sizeof(uint32_t); // Name count

		for (const auto& subMesh : mesh->mySubMeshes)
		{
			size += sizeof(uint32_t); // Name size
			size += subMesh.name.size();
		}

		// Sub meshes
		for (const auto& subMesh : mesh->mySubMeshes)
		{
			subMesh;

			size += sizeof(uint32_t); // Material index
			size += sizeof(uint32_t); // Vertex count
			size += sizeof(uint32_t); // Index count
			size += sizeof(uint32_t); // Vertex start offset
			size += sizeof(uint32_t); // Index start offset
			size += sizeof(glm::mat4); // Sub mesh transform
		}

		return size;
	}

	void MeshCompiler::CreateMaterial(Ref<Mesh> mesh, const std::filesystem::path& destination)
	{
		mesh->myMaterial->path = destination.parent_path().string() + "\\" + destination.stem().string() + ".vtmat";
		AssetManager::Get().SaveAsset(mesh->myMaterial);
	}
}
