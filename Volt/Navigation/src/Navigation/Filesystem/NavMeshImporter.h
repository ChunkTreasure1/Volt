#pragma once

#include "Navigation/NavMesh/VTNavMesh.h"

#include <fstream>

namespace Volt
{
	namespace AI
	{
		class NavMeshImporter
		{
		public:
			NavMeshImporter() = delete;
			virtual ~NavMeshImporter() = delete;
			
			static bool SaveNavMesh(std::ostream& output, Ref<NavMesh>& asset);
			static bool LoadNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset);

		private:
			static bool SaveSingleNavMesh(std::ostream& output, Ref<NavMesh>& asset);
			static bool SaveTiledNavMesh(std::ostream& output, Ref<NavMesh>& asset);

			static bool LoadSingleNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset);
			static bool LoadTiledNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset);
		};
	}
}
