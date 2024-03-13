#pragma once

#include "Navigation/NavMesh/VTNavMesh.h"

#include <fstream>

namespace Volt
{
	class BinaryStreamReader;
	class BinaryStreamWriter;

	namespace AI
	{
		class NavMeshImporter
		{
		public:
			NavMeshImporter() = delete;
			virtual ~NavMeshImporter() = delete;
			
			static bool SaveNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset);
			static bool LoadNavMesh(BinaryStreamReader& input, Ref<dtNavMesh>& asset);

			static bool LoadNavMeshLegacy(std::ifstream& input, Ref<dtNavMesh>& asset);

		private:
			struct NavMeshSetHeader
			{
				int magic;
				int version;
				int numTiles;
				dtNavMeshParams params;
			};

			struct NavMeshTileHeader
			{
				dtTileRef tileRef;
				int dataSize;
			};

			static bool SaveSingleNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset);
			static bool SaveTiledNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset);

			static bool LoadSingleNavMesh(BinaryStreamReader& input, const NavMeshSetHeader& header, Ref<dtNavMesh>& asset);
			static bool LoadTiledNavMesh(BinaryStreamReader& input, const NavMeshSetHeader& header, Ref<dtNavMesh>& asset);
		
			static bool LoadSingleNavMeshLegacy(std::ifstream& input, Ref<dtNavMesh>& asset);
		};
	}
}
