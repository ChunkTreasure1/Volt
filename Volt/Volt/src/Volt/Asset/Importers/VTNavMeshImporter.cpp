#include "vtpch.h"
#include "VTNavMeshImporter.h"
#include "VTMeshImporter.h"

#include "Volt/AI/NavMesh.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/MeshCompiler.h"
#include "Volt/Core/Profiling.h"

namespace Volt
{
	size_t CalculateNavMeshSize(Ref<NavMesh> navmesh)
	{
		// NOTE: This does not count in vtmesh data

		const auto navmeshData = navmesh->GetNavMeshData();

		size_t size = 0;

		size += sizeof(uint32_t); // Vertex Count

		size += sizeof(Pathfinder::vec3) * navmeshData.getVertCount(); // Vertices

		size += sizeof(uint32_t); // Polygon Count

		const uint32_t polyCount = navmeshData.getPolyCount();
		const auto polygons = navmeshData.getPolygons();
		for (uint32_t i = 0; i < polyCount; i++) // Polygons
		{
			size += sizeof(uint32_t); // Poly Vert Count
			size += sizeof(uint32_t) * polygons[i].verts.size();
			
			size += sizeof(uint32_t); // Poly Link Count
			size += sizeof(Pathfinder::pfLink) * polygons[i].links.size();
		}

		return size;
	}

	bool VTNavMeshImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		VT_PROFILE_FUNCTION();

		VTMeshImporter vtMeshImporter;
		auto vtmesh = vtMeshImporter.ImportMesh(path);
		if (!vtmesh)
		{
			return false;
		}

		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Could not open navmesh file!");
		}

		auto vertices = vtmesh->GetVertices();
		auto indices = vtmesh->GetIndices();
		auto material = AssetManager::GetAsset<Material>(NavMesh::navmeshMaterialPath);

		Pathfinder::pfNavMesh navmesh;

		std::vector<uint8_t> totalData;
		const size_t srcSize = file.seekg(0, std::ios::end).tellg();
		totalData.resize(srcSize);
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		file.close();

		size_t offset = MeshCompiler::CalculateMeshSize(vtmesh);

		const uint32_t vertexCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		std::vector<Pathfinder::vec3> verts;
		verts.resize(vertexCount);
		memcpy_s(verts.data(), sizeof(Pathfinder::vec3) * vertexCount, &totalData[offset], sizeof(Pathfinder::vec3) * vertexCount);
		navmesh.setVertices(verts);
		offset += sizeof(Pathfinder::vec3) * vertexCount;

		const uint32_t polyCount = *(uint32_t*)&totalData[offset];
		offset += sizeof(uint32_t);

		std::vector<Pathfinder::pfPoly> polys;
		polys.resize(polyCount);

		for (uint32_t i = 0; i < polyCount; i++)
		{
			const uint32_t polyVertCount = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			polys[i].verts.resize(polyVertCount);
			memcpy_s(polys[i].verts.data(), sizeof(uint32_t) * polyVertCount, &totalData[offset], sizeof(uint32_t) * polyVertCount);
			offset += sizeof(uint32_t) * polyVertCount;

			const uint32_t polyLinkCount = *(uint32_t*)&totalData[offset];
			offset += sizeof(uint32_t);

			polys[i].links.resize(polyLinkCount);
			memcpy_s(polys[i].links.data(), sizeof(Pathfinder::pfLink) * polyLinkCount, &totalData[offset], sizeof(Pathfinder::pfLink) * polyLinkCount);
			offset += sizeof(Pathfinder::pfLink) * polyLinkCount;
		}
		navmesh.setPolygons(polys);

		asset = CreateRef<NavMesh>(vertices, indices, material, navmesh);

		return true;
	}

	void VTNavMeshImporter::Save(const Ref<Asset>& asset) const
	{
		auto vtnavmesh = reinterpret_pointer_cast<NavMesh>(asset);
		MeshCompiler::TryCompile(vtnavmesh, vtnavmesh->path, vtnavmesh->GetMaterial()->handle);

		std::ofstream output(vtnavmesh->path, std::ios::binary | std::ios::app);
		auto navmeshData = vtnavmesh->GetNavMeshData();

		std::vector<uint8_t> bytes;
		bytes.resize(CalculateNavMeshSize(vtnavmesh));

		size_t offset = 0;

		const uint32_t vertexCount = navmeshData.getVertCount();
		memcpy_s(&bytes[offset], sizeof(uint32_t), &vertexCount, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		memcpy_s(&bytes[offset], sizeof(Pathfinder::vec3) * vertexCount, navmeshData.getVertices().data(), sizeof(Pathfinder::vec3) * vertexCount);
		offset += sizeof(Pathfinder::vec3) * vertexCount;

		const uint32_t polyCount = navmeshData.getPolyCount();
		memcpy_s(&bytes[offset], sizeof(uint32_t), &polyCount, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		const auto polygons = navmeshData.getPolygons();

		for (uint32_t i = 0; i < polyCount; i++)
		{
			const uint32_t polyVertCount = (uint32_t)polygons[i].verts.size();
			memcpy_s(&bytes[offset], sizeof(uint32_t), &polyVertCount, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy_s(&bytes[offset], sizeof(uint32_t) * polyVertCount, polygons[i].verts.data(), sizeof(uint32_t) * polyVertCount);
			offset += sizeof(uint32_t) * polyVertCount;

			const uint32_t polyLinkCount = (uint32_t)polygons[i].links.size();
			memcpy_s(&bytes[offset], sizeof(uint32_t), &polyLinkCount, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy_s(&bytes[offset], sizeof(Pathfinder::pfLink) * polyLinkCount, polygons[i].links.data(), sizeof(Pathfinder::pfLink) * polyLinkCount);
			offset += sizeof(Pathfinder::pfLink) * polyLinkCount;
		}

		output.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
		output.close();
	}

	void VTNavMeshImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
	{

	}

	bool VTNavMeshImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
	{
		return false;
	}
}