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
	bool VTMeshImporter::ImportMeshImpl(const std::filesystem::path& path, Mesh& dstMesh)
	{
		if (!std::filesystem::exists(path))
		{
			VT_CORE_ERROR("File does not exist: {0}", path.string().c_str());
			return false;
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

		size_t offset = 0;

		const uint32_t subMeshCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		const AssetHandle materialHandle = *(AssetHandle*)&totalData[offset];
		dstMesh.myMaterial = AssetManager::GetAsset<Material>(materialHandle);
		offset += sizeof(AssetHandle);

		const uint32_t vertexCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		dstMesh.myVertices.resize(vertexCount);
		memcpy_s(dstMesh.myVertices.data(), sizeof(Vertex) * vertexCount, &totalData[offset], sizeof(Vertex) * vertexCount);
		offset += sizeof(Vertex) * vertexCount;

		const uint32_t indexCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		dstMesh.myIndices.resize(indexCount);
		memcpy_s(dstMesh.myIndices.data(), sizeof(uint32_t) * indexCount, &totalData[offset], sizeof(uint32_t) * indexCount);
		offset += sizeof(uint32_t) * indexCount;

		if (!IsValid(subMeshCount, vertexCount, indexCount, srcSize) && path.extension() != ".vtnavmesh")
		{
			VT_CORE_ERROR("Mesh {0} is invalid! It needs to be recompiled!", path.string());
			dstMesh.SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		dstMesh.myBoundingSphere.center = *(glm::vec3*)&totalData[offset];
		offset += sizeof(glm::vec3);

		dstMesh.myBoundingSphere.radius = *(float*)&totalData[offset];
		offset += sizeof(float);

		const uint32_t nameCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		std::vector<std::string> names;
		names.reserve(nameCount);

		for (uint32_t i = 0; i < subMeshCount; i++)
		{
			const uint32_t nameSize = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			const char* nameData = (const char*)&totalData[offset];
			names.emplace_back(nameData);

			offset += nameSize;
		}

		for (uint32_t i = 0; i < subMeshCount; i++)
		{
			auto& subMesh = dstMesh.mySubMeshes.emplace_back();

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

			subMesh.transform = *(glm::mat4*)&totalData[offset];
			offset += sizeof(glm::mat4);

			if (i < (uint32_t)names.size())
			{
				subMesh.name = names.at(i);
			}

			subMesh.GenerateHash();
		}

		if (!dstMesh.myMaterial)
		{
			dstMesh.myMaterial = CreateRef<Material>();
			dstMesh.myMaterial->mySubMaterials.emplace(0, SubMaterial::Create("Null", 0, Renderer::GetDefaultData().defaultShader));
		}

		dstMesh.Construct();

		return true;
	}

	bool VTMeshImporter::IsValid(uint32_t subMeshCount, uint32_t vertexCount, uint32_t indexCount, size_t srcSize) const
	{
		size_t staticSize = sizeof(uint32_t); // Sub mesh count
		staticSize += sizeof(AssetHandle); // AssetHandle
		staticSize += sizeof(uint32_t); // Vertex count
		staticSize += sizeof(uint32_t); // Index count
		staticSize += sizeof(glm::vec3); // Bounding Sphere center
		staticSize += sizeof(float); // Bounding Sphere radius
		staticSize += sizeof(uint32_t); // Name count

		struct SubMesh
		{
			uint32_t materialIndex = 0;
			uint32_t vertexCount = 0;
			uint32_t indexCount = 0;
			uint32_t vertexStartOffset = 0;
			uint32_t indexStartOffset = 0;
			glm::mat4 transform = { 1.f };
		};

		size_t dynamicSize = sizeof(SubMesh) * subMeshCount;
		dynamicSize += sizeof(Vertex) * vertexCount;
		dynamicSize += sizeof(uint32_t) * indexCount;

		const size_t totalSize = dynamicSize + staticSize;

		return srcSize >= totalSize;
	}
}
