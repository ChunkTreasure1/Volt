#include "nvpch.h"
#include "VTNavMesh.h"

#include <Volt/Log/Log.h>

namespace Volt
{
	namespace AI
	{
		NavMesh::NavMesh(Ref<dtNavMesh> navmesh, Ref<dtTileCache> tilecache)
		{
			myNavMesh = CreateRef<DtNavMesh>(navmesh, tilecache);
			myCrowd = CreateRef<DtCrowd>(myNavMesh);
		}

		void NavMesh::Update(float deltaTime)
		{
			if (myNavMesh)
			{
				if (!myCrowd) return;

				myCrowd->GetDTCrowd()->update(deltaTime, nullptr);
			}
		}
	}
}
