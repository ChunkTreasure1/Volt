#include "vtpch.h"
#include "NavMeshGenerator.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/NavigationComponents.h"
#include "Volt/Components/Components.h"

#include <gem/gem.h>
#include <unordered_map>
#include <utility>
#include <bitset>

namespace Volt
{
	Volt::MeshInfo NavMeshGenerator::GetNavMeshVertices(const Volt::Entity aEntity, const float& aAngle)
	{
		MeshInfo result;

		auto& meshComp = aEntity.GetComponent<Volt::MeshComponent>();
		auto& transformComp = aEntity.GetComponent<Volt::TransformComponent>();

		if (meshComp.walkable)
		{
			auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle);
			auto vertexList = mesh->GetVertices();
			auto indexList = mesh->GetIndices();

			for (auto& v : vertexList)
			{
				gem::vec4 v4(v.position);
				v4.w = 1.f;
				v.position = transformComp.GetTransform() * v4;
			}

			MeshInfo info;
			info.first = vertexList;
			info.second = indexList;

			auto up = aEntity.GetLocalUp();
			auto final = FilterNavMeshVertices(info, up, aAngle);

			result.first.insert(result.first.end(), final.first.begin(), final.first.end());
			result.second.insert(result.second.end(), final.second.begin(), final.second.end());
		}

		return result;
	}

	MeshInfo NavMeshGenerator::FilterNavMeshVertices(const MeshInfo& aNavMeshList, const gem::vec3& aUpDir, const float& aAngle, const uint32_t& aIndexOffset)
	{
		MeshInfo result;

		std::unordered_map<uint32_t, uint32_t> indexMap;
		std::vector<uint32_t> usableIndexes;

		for (int i = 0; i < aNavMeshList.first.size(); i++)
		{
			if (gem::dot(gem::normalize(aUpDir), gem::normalize(aNavMeshList.first[i].normal)) >= gem::cos(gem::radians(aAngle)) + 0.01f)
			{
				result.first.push_back(aNavMeshList.first[i]);
				indexMap[i] = (uint32_t)usableIndexes.size();
				usableIndexes.push_back(i);
			}
		}

		result.second = CalculateUsableIndices(aNavMeshList.second, usableIndexes, indexMap, aIndexOffset);
		if (result.second.size() == 0)
		{
			result.first.clear();
		}

		return result;
	}

	std::vector<uint32_t> NavMeshGenerator::CalculateUsableIndices(const std::vector<uint32_t>& aIndexList, const std::vector<uint32_t>& aUsableIndices, const std::unordered_map<uint32_t, uint32_t>& aIndexToUsableMap, const uint32_t& aIndexOffset)
	{
		std::vector<uint32_t> result;

		for (int j = 0; j < aIndexList.size(); j += 3)
		{
			if (TriangulationUtils::contains(aUsableIndices, aIndexList[j]) &&
				TriangulationUtils::contains(aUsableIndices, aIndexList[j + 1]) &&
				TriangulationUtils::contains(aUsableIndices, aIndexList[j + 2]))
			{
				result.push_back(aIndexToUsableMap.find(aIndexList[j])->second + aIndexOffset);
				result.push_back(aIndexToUsableMap.find(aIndexList[j + 1])->second + aIndexOffset);
				result.push_back(aIndexToUsableMap.find(aIndexList[j + 2])->second + aIndexOffset);
			}
		}

		return result;
	}

	bool CheckNeighbourConditions(const Volt::NavMeshCell& aFirstCell, const Volt::NavMeshCell& aSecondCell)
	{
		uint16_t sharedCount = 0;
		for (const auto& firstIndex : aFirstCell.Indices)
		{
			for (const auto& secondIndex : aSecondCell.Indices)
			{
				if (firstIndex == secondIndex)
				{
					sharedCount++;
					break;
				}
			}
		}
		return (sharedCount > 1) ? true : false;
	}

	std::pair<uint32_t, uint32_t> GetPortals(const Volt::NavMeshCell& a, const Volt::NavMeshCell& b)
	{
		std::pair<uint32_t, uint32_t> result;
		result.first = 0;
		result.second = 0;

		for (const auto& indexA : a.Indices)
		{
			for (const auto& indexB : b.Indices)
			{
				if (indexA == indexB)
				{
					if (!result.first)
					{
						result.first = indexA;
					}
					else
					{
						result.second = indexA;
						return result;
					}
				}
			}
		}

		return result;
	}

	std::vector<Volt::NavMeshCell> NavMeshGenerator::GenerateNavMeshCells(Ref<Volt::NavMesh> aNavMesh)
	{
		std::vector<Volt::NavMeshCell> result;

		for (uint32_t i = 0; i < aNavMesh->Indices.size(); i += 3)
		{
			Volt::NavMeshCell cell;
			cell.Indices = { aNavMesh->Indices[i], aNavMesh->Indices[i + 1], aNavMesh->Indices[i + 2] };
			cell.Center = (aNavMesh->Vertices[cell.Indices[0]].Position + aNavMesh->Vertices[cell.Indices[1]].Position + aNavMesh->Vertices[cell.Indices[2]].Position) / 3.f;

			result.push_back(cell);
			result.back().Id = (Volt::CellID)result.size();
		}

		for (auto& cell : result)
		{
			if (cell.NeighbourCells[0] != 0 && cell.NeighbourCells[1] != 0 && cell.NeighbourCells[2] != 0) { continue; }

			for (const auto& checkCell : result)
			{
				if (cell.Id == checkCell.Id) { continue; }
				if (CheckNeighbourConditions(cell, checkCell))
				{
					for (uint32_t i = 0; i < cell.NeighbourCells.size(); i++)
					{
						if (cell.NeighbourCells[i] == 0)
						{
							cell.NeighbourCells[i] = checkCell.Id;
							cell.numNeighbours++;
							break;
						}
					}
				}
			}
		}

		for (auto& cell : result)
		{
			for (uint32_t i = 0; i < cell.NeighbourCells.size(); i++)
			{
				if (cell.NeighbourCells[i] == 0) { break; }
				auto neighbour = result[cell.NeighbourCells[i] - 1];
				auto portals = GetPortals(cell, neighbour);
				if (portals.first || portals.second)
				{
					cell.NeighbourPortals[i] = (aNavMesh->Vertices[portals.first].Position + aNavMesh->Vertices[portals.second].Position) / 2.f;
				}
			}
		}

		return result;
	}

	std::vector<Volt::Entity> NavMeshGenerator::GetModifierVolumes(Ref<Volt::Scene> aScene)
	{
		std::vector<Volt::Entity> result;

		aScene->GetRegistry().ForEach<Volt::NavMeshModifierComponent>([&](Wire::EntityId id, Volt::NavMeshModifierComponent& modiferComp)
			{
				if (modiferComp.active)
				{
					result.emplace_back(Entity(id, aScene.get()));
				}
			});

		return result;
	}
}
