#include "vtpch.h"
#include "Volt/Asset/Mesh/MeshCompiler.h"

#include "Volt/Core/BinarySerializer.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Project/ProjectManager.h"

#include <AssetSystem/AssetManager.h>

#include <fstream>

namespace Volt
{
	bool MeshCompiler::TryCompile(Ref<Mesh> mesh, const std::filesystem::path& destination, const MaterialTable& materialTable)
	{
		if (!mesh || !mesh->IsValid())
		{
			VT_LOG(Error, "Mesh is invalid or null!");
			return false;
		}

		if (!materialTable.IsValid())
		{
			SetupMaterials(mesh, destination);
		}
		else
		{
			mesh->m_materialTable = materialTable;
		}

		const auto& tempMaterialTable = mesh->m_materialTable;

		/*
		* Encoding:
		* uint32_t: Sub mesh count
		* uint32_t: Material count
		* material handles
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

		BinarySerializer serializer{ Volt::ProjectManager::GetRootDirectory() / destination, CalculateMeshSize(mesh) };

		const uint32_t submeshCount = (uint32_t)mesh->m_subMeshes.size();
		const uint32_t vertexCount = (uint32_t)mesh->m_vertexContainer.positions.size();
		const uint32_t indexCount = (uint32_t)mesh->m_indices.size();

		const glm::vec3 boundingCenter = mesh->m_boundingSphere.center;
		const float boundingRadius = mesh->m_boundingSphere.radius;

		serializer.Serialize<uint32_t>(submeshCount);
		serializer.Serialize<uint32_t>(tempMaterialTable.GetSize());

		for (const auto& mat : tempMaterialTable)
		{
			serializer.Serialize<AssetHandle>(mat);
		}

		serializer.Serialize<uint32_t>(vertexCount);
		//serializer.Serialize(mesh->m_vertices.data(), sizeof(Vertex) * vertexCount);

		serializer.Serialize<uint32_t>(indexCount);
		serializer.Serialize(mesh->m_indices.data(), sizeof(uint32_t) * indexCount);

		serializer.Serialize<glm::vec3>(boundingCenter);
		serializer.Serialize<float>(boundingRadius);

		serializer.Serialize<uint32_t>(submeshCount);

		for (const auto& subMesh : mesh->m_subMeshes)
		{
			serializer.Serialize<uint32_t>((uint32_t)subMesh.name.size());
			serializer.Serialize<std::string>(subMesh.name);
		}

		for (const auto& subMesh : mesh->m_subMeshes)
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
		size += sizeof(uint32_t); // Material handle
		size += sizeof(AssetHandle) * mesh->m_materialTable.GetSize();

		size += sizeof(uint32_t); // Vertex count
		//size += sizeof(Vertex) * mesh->m_vertices.size(); // Vertices
		size += sizeof(uint32_t); // Index count
		size += sizeof(uint32_t) * mesh->m_indices.size(); // Indices

		size += sizeof(glm::vec3); // Bounding sphere center
		size += sizeof(float); // Bounding sphere radius

		// Sub mesh names
		size += sizeof(uint32_t); // Name count

		for (const auto& subMesh : mesh->m_subMeshes)
		{
			size += sizeof(uint32_t); // Name size
			size += subMesh.name.size();
		}

		// Sub meshes
		for (const auto& subMesh : mesh->m_subMeshes)
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

	void MeshCompiler::SetupMaterials(Ref<Mesh> mesh, const std::filesystem::path& destination)
	{
		const auto materialPath = destination.parent_path();

		for (auto& material : mesh->m_materialTable)
		{
			auto asset = AssetManager::GetAsset<Material>(material);

			AssetManager::Get().MoveAsset(asset, materialPath);
			AssetManager::SaveAsset(asset);
		}
	}
}
