#pragma once

#include "Volt/Asset/Mesh/Mesh.h"
#include "Pathfinder/pfNavMesh.h"

namespace Volt
{
	Pathfinder::vec3 VTtoPF(gem::vec3 x);
	gem::vec3 PFtoVT(Pathfinder::vec3 x);

	class VertexBuffer;
	class IndexBuffer;
	class Material;

	class NavMesh : public Mesh
	{
	public:
		NavMesh() = default;
		NavMesh(std::vector<Vertex> aVertices, std::vector<uint32_t> aIndices, Ref<Material>& aMaterial, Pathfinder::pfNavMesh aNavMesh) : Mesh(aVertices, aIndices, aMaterial), myNavMesh(aNavMesh) {};
		~NavMesh() override = default;

		const Pathfinder::pfNavMesh& GetNavMeshData() { return myNavMesh; };

		static AssetType GetStaticType() { return AssetType::NavMesh; }
		AssetType GetType() override { return GetStaticType(); }

		inline static std::filesystem::path navmeshMaterialPath = "Editor/Materials/M_NavMeshDebug.vtmat";

	private:
		Pathfinder::pfNavMesh myNavMesh;
	};
}