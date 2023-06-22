#pragma once

#include <Volt/Core/Base.h>

#include <glm/glm.hpp>

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourTileCache.h>

#include <vector>

namespace Volt
{
	namespace AI
	{
		class DtNavMesh
		{
		public:
			DtNavMesh(Ref<dtNavMesh> navmesh, Ref<dtTileCache> tilecache = nullptr);

			std::vector<glm::vec3> FindPath(glm::vec3 start, glm::vec3 end, glm::vec3 polySearchDistance);

			Ref<dtNavMesh>& GetNavMesh() { return myNavMesh; };
			Ref<dtNavMeshQuery>& GetNavMeshQuery() { return myNavMeshQuery; };
			Ref<dtTileCache>& GetTileCache() { return myTileCache; };

		private:
			Ref<dtNavMesh> myNavMesh = nullptr;
			Ref<dtNavMeshQuery> myNavMeshQuery = nullptr;
			Ref<dtTileCache> myTileCache = nullptr;
		};
	}
}
