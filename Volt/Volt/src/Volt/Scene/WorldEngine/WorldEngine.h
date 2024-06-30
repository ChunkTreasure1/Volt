#pragma once

#include "WorldCell.h"

namespace Volt
{
	struct WorldGridSettings
	{
		int32_t cellSize = 25'600;
		glm::uvec2 worldSize = { 128'000, 128'000 };
	};

	class Scene;
	class Entity;

	class WorldEngine
	{
	public:
		WorldEngine() = default;

		void Reset(Scene* scene, uint32_t initialCellCount, uint32_t cellCountWidth);
		void AddEntity(const Entity& entity);
		void RemoveEntity(const Entity& entity);
		void AddCell(const glm::ivec3& origin, WorldCellID id = std::numeric_limits<uint32_t>::max());
		void GenerateCells();
		void AddEntitiesToCell(WorldCellID cellId, const std::vector<EntityID>& entities);
		void OnEntityMoved(const Entity& entity);

		void BeginStreamingCell(WorldCellID cellId);

		WorldCellID GetCellIDFromEntity(const Entity& entity) const;
		[[nodiscard]] bool IsCellLoaded(WorldCellID cellId) const;

		[[nodiscard]] inline const std::vector<WorldCell>& GetCells() const { return m_cells; }
		[[nodiscard]] inline const WorldGridSettings& GetSettings() const { return m_settings; }
		[[nodiscard]] inline WorldGridSettings& GetSettingsMutable() { return m_settings; }

	private:
		WorldCell& GetCellFromID(WorldCellID cellId);
		const WorldCell& GetCellFromID(WorldCellID cellId) const;

		WorldGridSettings m_settings;
		std::vector<WorldCell> m_cells;

		Scene* m_scene = nullptr;
	};
}
