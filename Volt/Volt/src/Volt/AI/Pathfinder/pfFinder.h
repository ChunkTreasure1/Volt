#pragma once
#include "Math/pfMath.h"

#include <unordered_map>

namespace Pathfinder
{
	class pfNavMesh;
	class pfLink;

	class pfFinder
	{
	public:
		pfFinder(const pfNavMesh* navmesh);
		~pfFinder() = default;

		std::vector<uint32_t> AStar(uint32_t start, uint32_t end);
		std::vector<vec3> Funnel(const std::vector<vec3>& path, const std::vector<pfLink>& portals);

	private:
		void reconstruct_path(uint32_t current, std::vector<uint32_t>& path);
		float Cost(uint32_t from, uint32_t to);

		std::unordered_map<uint32_t, float> m_gScore;
		std::unordered_map<uint32_t, float> m_fScore;
		std::unordered_map<uint32_t, uint32_t> m_parents;

		const pfNavMesh* m_navmesh;
	};
}