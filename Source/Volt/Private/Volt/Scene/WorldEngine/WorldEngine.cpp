#include "vtpch.h"
#include "Volt/Scene/WorldEngine/WorldEngine.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/CoreComponents.h"

#include "Volt/Asset/Serializers/SceneSerializer.h"

#include "Volt/Math/Math.h"

#include <JobSystem/JobSystem.h>

namespace Volt
{
	void WorldEngine::Reset(Scene* scene, uint32_t initialCellCount, uint32_t cellCountWidth)
	{
		m_scene = scene;

		//m_cells.clear();

		//for (uint32_t i = 0; i < initialCellCount; i++)
		//{
		//	auto& newCell = m_cells.emplace_back();
		//	newCell.cellId = i;
		//	newCell.origin = { (i / cellCountWidth) * m_settings.cellSize / 2, 0, (i % cellCountWidth) * m_settings.cellSize / 2 };
		//}

		//// Put entities into cells
		//for (const auto& entity : m_scene->GetAllEntities())
		//{
		//	AddEntity(entity);
		//}

		GenerateCells();
	}

	void WorldEngine::AddEntity(const Entity& entity)
	{
		// We only want to add non movable actors to World Engine
		if (entity.GetComponent<TransformComponent>().movability == Movability::Movable)
		{
			return;
		}

		const auto position = entity.GetPosition();
		const glm::ivec3 intPosition = position;

		const int32_t closestXPos = Math::RoundToClosestMultiple(intPosition.x, m_settings.cellSize / 2);
		const int32_t closestZPos = Math::RoundToClosestMultiple(intPosition.z, m_settings.cellSize / 2);

		for (auto& cell : m_cells)
		{
			if (cell.origin == glm::ivec3{ closestXPos, 0, closestZPos })
			{
				cell.cellEntities.emplace_back(entity.GetID());
			}
		}
	}

	void WorldEngine::RemoveEntity(const Entity& entity)
	{
		for (auto& cell : m_cells)
		{
			auto it = std::find_if(cell.cellEntities.begin(), cell.cellEntities.end(), [&](const auto id) { return id == entity.GetID(); });
			if (it != cell.cellEntities.end())
			{
				cell.cellEntities.erase(it);
			}
		}
	}

	void WorldEngine::AddCell(const glm::ivec3& origin, WorldCellID id)
	{
		for (const auto& cell : m_cells)
		{
			if (cell.origin == origin)
			{
				// Cell with that origin already exists!
				return;
			}
		}

		auto& newCell = m_cells.emplace_back();
		newCell.origin = origin;
		newCell.cellId = id == std::numeric_limits<uint32_t>::max() ? static_cast<WorldCellID>(m_cells.size() - 1) : id;
	}

	void WorldEngine::GenerateCells()
	{
		m_cells.clear();

		const glm::uvec2& worldSize = m_settings.worldSize;

		// Find absolute origin offset
		const glm::ivec2 absoluteOrigin = worldSize / 2u;

		const uint32_t cellCountX = Math::DivideRoundUp(worldSize.x, static_cast<uint32_t>(m_settings.cellSize));
		const uint32_t cellCountY = Math::DivideRoundUp(worldSize.y, static_cast<uint32_t>(m_settings.cellSize));

		for (uint32_t x = 0; x < cellCountX; x++)
		{
			for (uint32_t y = 0; y < cellCountY; y++)
			{
				const glm::ivec3 cellOrigin = { x * m_settings.cellSize - absoluteOrigin.x + m_settings.cellSize / 2, 0, y * m_settings.cellSize - absoluteOrigin.y + m_settings.cellSize / 2 };
				AddCell(cellOrigin);
			}
		}

		// Put entities into cells
		for (const auto& entity : m_scene->GetAllEntities())
		{
			AddEntity(entity);
		}
	}

	void WorldEngine::AddEntitiesToCell(WorldCellID cellId, const Vector<EntityID>& entities)
	{
		auto cellIt = std::find_if(m_cells.begin(), m_cells.end(), [cellId](const WorldCell& cell) 
		{
			return cell.cellId == cellId;
		});

		if (cellIt == m_cells.end())
		{
			return;
		}

		WorldCell& cell = *cellIt;
		cell.cellEntities.insert(cell.cellEntities.end(), entities.begin(), entities.end());
	}

	void WorldEngine::OnEntityMoved(const Entity& entity)
	{
		RemoveEntity(entity);
		AddEntity(entity);
	}

	void WorldEngine::BeginStreamingCell(WorldCellID cellId)
	{
		auto& cell = GetCellFromID(cellId);
		if (cell.cellId == INVALID_WORLD_CELL_ID)
		{
			return;
		}

		JobSystem::SubmitTask([cellId, this]() 
		{
			auto& cell = GetCellFromID(cellId);

			SceneSerializer::Get().LoadWorldCell(m_scene->shared_from_this(), cell);
			cell.isLoaded = true;
		});
	}

	WorldCellID WorldEngine::GetCellIDFromEntity(const Entity& entity) const
	{
		for (const auto& cell : m_cells)
		{
			auto it = std::find_if(cell.cellEntities.begin(), cell.cellEntities.end(), [&](const auto id) { return id == entity.GetID(); });
			if (it != cell.cellEntities.end())
			{
				return cell.cellId;
			}
		}

		return INVALID_WORLD_CELL_ID;
	}

	bool WorldEngine::IsCellLoaded(WorldCellID cellId) const
	{
		const auto& cell = GetCellFromID(cellId);
		return cell.isLoaded;
	}

	WorldCell& WorldEngine::GetCellFromID(WorldCellID cellId)
	{
		for (auto& cell : m_cells)
		{
			if (cell.cellId == cellId)
			{
				return cell;
			}
		}

		static WorldCell nullCell{};
		return nullCell;
	}

	const WorldCell& WorldEngine::GetCellFromID(WorldCellID cellId) const
	{
		for (auto& cell : m_cells)
		{
			if (cell.cellId == cellId)
			{
				return cell;
			}
		}

		static WorldCell nullCell{};
		return nullCell;
	}
}
