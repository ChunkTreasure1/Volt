#include "nvpch.h"
#include "DtNavMesh.h"

#include "Navigation/Core/CoreInterfaces.h"

#include <Volt/Log/Log.h>

namespace Volt
{
	namespace AI
	{
		DtNavMesh::DtNavMesh(Ref<dtNavMesh> navmesh, Ref<dtTileCache> tilecache)
			: myNavMesh(navmesh), myTileCache(tilecache)
		{
			dtStatus status;

			myNavMeshQuery = CreateRef<dtNavMeshQuery>();

			status = myNavMeshQuery->init(myNavMesh.get(), 2048);
			if (dtStatusFailed(status))
			{
				VT_CORE_ERROR("Could not init Detour NavMesh Query");
			}
		}

		std::vector<gem::vec3> DtNavMesh::FindPath(gem::vec3 start, gem::vec3 end, gem::vec3 polySearchDistance)
		{
			std::vector<gem::vec3> resultPath;

			if (!myNavMesh || !myNavMeshQuery) { return resultPath; }

			dtStatus status = 0;

			dtQueryFilter dtFilter;
			dtFilter.setIncludeFlags(POLYFLAGS_ALL ^ POLYFLAGS_DISABLED);
			dtFilter.setExcludeFlags(0);

			// Change costs.
			dtFilter.setAreaCost(POLYAREA_GROUND, 1.0f);
			//dtFilter.setAreaCost(POLYAREA_WATER, 10.0f);
			//dtFilter.setAreaCost(POLYAREA_ROAD, 1.0f);
			//dtFilter.setAreaCost(POLYAREA_DOOR, 1.0f);
			//dtFilter.setAreaCost(POLYAREA_GRASS, 2.0f);
			//dtFilter.setAreaCost(POLYAREA_JUMP, 1.5f);

			gem::vec3 halfExtents = polySearchDistance;

			dtPolyRef startPoly;
			gem::vec3 startPoint;

			status = myNavMeshQuery->findNearestPoly((const float*)&start, (const float*)&halfExtents, &dtFilter, &startPoly, (float*)&startPoint);

			if (dtStatusFailed(status))
			{
				VT_CORE_WARN("Detour failed to find nearest start poly");
				return resultPath;
			}

			dtPolyRef endPoly;
			gem::vec3 endPoint;

			status = myNavMeshQuery->findNearestPoly((const float*)&end, (const float*)&halfExtents, &dtFilter, &endPoly, (float*)&endPoint);

			if (dtStatusFailed(status))
			{
				VT_CORE_WARN("Detour failed to find nearest end poly");
				return resultPath;
			}

			const int MAX_PATH_SIZE = 25;

			dtPolyRef srcPath[sizeof(dtPolyRef) * MAX_PATH_SIZE];
			uint32_t srcPathSize = 0;

			status = myNavMeshQuery->findPath(startPoly, endPoly, (const float*)&startPoint, (const float*)&endPoint, &dtFilter, srcPath, (int*)&srcPathSize, MAX_PATH_SIZE);

			if (dtStatusFailed(status) || srcPathSize == 0)
			{
				VT_CORE_WARN("Detour failed to find path");
				return resultPath;
			}

			std::vector<gem::vec3> straightPathPositions;
			uint32_t straightPathSize = 0;

			straightPathPositions.resize(MAX_PATH_SIZE);

			status = myNavMeshQuery->findStraightPath((const float*)&startPoint, (const float*)&endPoint, srcPath, srcPathSize, (float*)straightPathPositions.data(), nullptr, nullptr, (int*)&straightPathSize, MAX_PATH_SIZE);

			if (dtStatusFailed(status))
			{
				VT_CORE_WARN("Detour failed to find straight path");
				return resultPath;
			}

			resultPath.resize(straightPathSize);
			memcpy_s(resultPath.data(), sizeof(gem::vec3) * straightPathSize, straightPathPositions.data(), sizeof(gem::vec3) * straightPathSize);
			return resultPath;
		}
	}
}
