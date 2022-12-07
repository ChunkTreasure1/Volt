#pragma once
#include "Volt/AI/NavMesh/NavMeshData.h"
#include "Volt/AI/NavMesh/Triangulation/Triangulation.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Scene/Scene.h"

#define VT_WORLD_UNIT 100.f

namespace Volt
{
	class NavMeshGenerator
	{
	public:
		NavMeshGenerator() = delete;
		~NavMeshGenerator() = delete;

		static MeshInfo GetNavMeshVertices(const Volt::Entity aEntity, const float& aAngle);
		static std::vector<Volt::NavMeshCell> GenerateNavMeshCells(Ref<Volt::NavMesh> aNavMesh);
		static std::vector<Volt::Entity> GetModifierVolumes(Ref<Volt::Scene> aScene);

	private:
		static MeshInfo FilterNavMeshVertices(const MeshInfo& aNavMeshList, const gem::vec3& aUpDir, const float& aAngle, const uint32_t& aIndexOffset = 0);
		static std::vector<uint32_t> CalculateUsableIndices(const std::vector<uint32_t>& aIndexList, const std::vector<uint32_t>& aUsableIndices, const std::unordered_map<uint32_t, uint32_t>& aIndexToUsableMap, const uint32_t& aIndexOffset = 0);
	};
}