#include "nvpch.h"
#include "VTNavMesh.h"

namespace Volt
{
	namespace AI
	{
		void NavMesh::Initialize(Ref<dtNavMesh> navmesh, Ref<dtTileCache> tilecache)
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
