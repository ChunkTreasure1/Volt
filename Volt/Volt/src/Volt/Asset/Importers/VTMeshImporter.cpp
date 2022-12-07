#include "vtpch.h"
#include "VTMeshImporter.h"

#include "Volt/Log/Log.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Rendering/Renderer.h"

namespace Volt
{
	Ref<Mesh> VTMeshImporter::ImportMeshImpl(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File does not exist: {0}", path.string().c_str());
			return nullptr;
		}

		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Could not open mesh file!");
		}

		std::vector<uint8_t> totalData;
		const size_t srcSize = file.seekg(0, std::ios::end).tellg();
		totalData.resize(srcSize);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		file.close();

		Ref<Mesh> mesh = CreateRef<Mesh>();

		size_t offset = 0;

		const uint32_t subMeshCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		const AssetHandle materialHandle = *(AssetHandle*)&totalData[offset];
		mesh->myMaterial = AssetManager::GetAsset<Material>(materialHandle);
		offset += sizeof(AssetHandle);

		const uint32_t vertexCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		mesh->myVertices.resize(vertexCount);
		memcpy_s(mesh->myVertices.data(), sizeof(Vertex) * vertexCount, &totalData[offset], sizeof(Vertex) * vertexCount);
		offset += sizeof(Vertex) * vertexCount;

		const uint32_t indexCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		mesh->myIndices.resize(indexCount);
		memcpy_s(mesh->myIndices.data(), sizeof(uint32_t) * indexCount, &totalData[offset], sizeof(uint32_t) * indexCount);
		offset += sizeof(uint32_t) * indexCount;

		if (!IsValid(subMeshCount, vertexCount, indexCount, srcSize))
		{
			VT_CORE_ERROR("Mesh {0} is invalid! It needs to be recompiled!", path.string());
			mesh->SetFlag(AssetFlag::Invalid, true);
			return mesh;
		}

		mesh->myBoundingSphere.center = *(gem::vec3*)&totalData[offset];
		offset += sizeof(gem::vec3);

		mesh->myBoundingSphere.radius = *(float*)&totalData[offset];
		offset += sizeof(float);

		for (uint32_t i = 0; i < subMeshCount; i++)
		{
			auto& subMesh = mesh->mySubMeshes.emplace_back();

			subMesh.materialIndex = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			subMesh.vertexCount = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			subMesh.indexCount = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			subMesh.vertexStartOffset = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			subMesh.indexStartOffset = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			//subMesh.transform = *(gem::mat4*)&totalData[offset];
			//offset += sizeof(gem::mat4);

			subMesh.GenerateHash();
		}

		if (!mesh->myMaterial)
		{
			mesh->myMaterial = CreateRef<Material>();
			mesh->myMaterial->mySubMaterials.emplace(0, SubMaterial::Create("Null", 0, Renderer::GetDefaultData().defaultShader));
		}

		for (auto& submesh : mesh->mySubMeshes)
		{
			if (submesh.materialIndex >= mesh->myMaterial->GetSubMaterials().size())
			{
				submesh.materialIndex = 0;
			}
		}

		mesh->path = path;
		mesh->Construct();

		return mesh;
	}

	bool VTMeshImporter::IsValid(uint32_t subMeshCount, uint32_t vertexCount, uint32_t indexCount, size_t srcSize) const
	{
		size_t staticSize = sizeof(uint32_t); // Sub mesh count
		staticSize += sizeof(AssetHandle); // AssetHandle
		staticSize += sizeof(uint32_t); // Vertex count
		staticSize += sizeof(uint32_t); // Index count
		staticSize += sizeof(gem::vec3); // Bounding Sphere center
		staticSize += sizeof(float); // Bounding Sphere radius

		struct SubMesh
		{
			uint32_t materialIndex = 0;
			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;
			uint32_t vertexStartOffset = 0;
			uint32_t indexStartOffset = 0;
			//gem::mat4 transform = { 1.f };
		};

		size_t dynamicSize = sizeof(SubMesh) * subMeshCount;
		dynamicSize += sizeof(Vertex) * vertexCount;
		dynamicSize += sizeof(uint32_t) * indexCount;

		const size_t totalSize = dynamicSize + staticSize;

		return srcSize == totalSize;
	} 
}