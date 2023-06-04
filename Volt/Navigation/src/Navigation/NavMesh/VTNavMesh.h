#pragma once

#include "Navigation/Core/CoreInterfaces.h"

#include "Navigation/NavMesh/DtNavMesh.h"
#include "Navigation/Crowd/DtCrowd.h"

#include <Volt/Asset/Asset.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/NavigationComponents.h>


namespace Volt
{
	namespace AI
	{
		class NavMesh : public Asset
		{
		public:
			NavMesh() = default;
			NavMesh(Ref<dtNavMesh> navmesh, Ref<dtTileCache> tilecache = nullptr);
			virtual ~NavMesh() = default;

			Ref<DtNavMesh>& GetNavMesh() { return myNavMesh; }
			Ref<DtCrowd>& GetCrowd() { return myCrowd; }

			static AssetType GetStaticType() { return AssetType::NavMesh; }
			AssetType GetType() override { return GetStaticType(); }

		private:
			friend class NavigationSystem;
			void Update(float deltaTime);

			Ref<DtNavMesh> myNavMesh = nullptr;
			Ref<DtCrowd> myCrowd = nullptr;
		};
	}
}
