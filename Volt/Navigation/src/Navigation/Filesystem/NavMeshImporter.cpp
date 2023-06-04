#include "nvpch.h"
#include "NavMeshImporter.h"

#include <Volt/Log/Log.h>
#include <Volt/Core/Profiling.h>

#include <Volt/Asset/AssetManager.h>

namespace Volt
{
	namespace AI
	{
		static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
		static const int NAVMESHSET_VERSION = 2;

		static const int TILECACHESET_MAGIC = 'T' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'TSET';
		static const int TILECACHESET_VERSION = 2;

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

		VT_OPTIMIZE_OFF

		bool NavMeshImporter::SaveNavMesh(std::ostream& output, Ref<NavMesh>& asset)
		{
			if (asset->GetNavMesh()->GetTileCache())
			{
				return SaveTiledNavMesh(output, asset);
			}
			else
			{
				return SaveSingleNavMesh(output, asset);
			}
		}

		bool NavMeshImporter::LoadNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset)
		{
			std::streampos currentPosition = input.tellg();

			// Read header.
			NavMeshSetHeader header;
			if (!input.read(reinterpret_cast<char*>(&header), sizeof(NavMeshSetHeader)))
			{
				input.close();
				return false;
			}

			input.seekg(currentPosition);

			switch (header.magic)
			{
				case NAVMESHSET_MAGIC:
				{
					return LoadSingleNavMesh(input, asset);
				}
				case TILECACHESET_MAGIC:
				{
					return LoadTiledNavMesh(input, asset);
				}
				default:
				{
					input.close();
					return false;
				}
			}
		}

		bool NavMeshImporter::SaveSingleNavMesh(std::ostream& output, Ref<NavMesh>& asset)
		{
			const auto* mesh = asset->GetNavMesh()->GetNavMesh().get();

			// Store header.
			NavMeshSetHeader header;
			header.magic = NAVMESHSET_MAGIC;
			header.version = NAVMESHSET_VERSION;
			header.numTiles = 0;
			for (int i = 0; i < mesh->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = mesh->getTile(i);
				if (!tile || !tile->header || !tile->dataSize) continue;
				header.numTiles++;
			}
			memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
			output.write(reinterpret_cast<char*>(&header), sizeof(NavMeshSetHeader));

			// Store tiles.
			for (int i = 0; i < mesh->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = mesh->getTile(i);
				if (!tile || !tile->header || !tile->dataSize) continue;

				NavMeshTileHeader tileHeader;
				tileHeader.tileRef = mesh->getTileRef(tile);
				tileHeader.dataSize = tile->dataSize;
				output.write(reinterpret_cast<char*>(&tileHeader), sizeof(NavMeshTileHeader));

				output.write(reinterpret_cast<const char*>(tile->data), tile->dataSize);
			}
			return true;
		}

		bool NavMeshImporter::SaveTiledNavMesh(std::ostream& output, Ref<NavMesh>& asset)
		{
			return true;
		}

		bool NavMeshImporter::LoadSingleNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset)
		{
			// Read header.
			NavMeshSetHeader header;
			if (!input.read(reinterpret_cast<char*>(&header), sizeof(NavMeshSetHeader)))
			{
				input.close();
				return false;
			}

			if (header.magic != NAVMESHSET_MAGIC)
			{
				input.close();
				return false;
			}
			if (header.version != NAVMESHSET_VERSION)
			{
				input.close();
				return false;
			}

			dtNavMesh* mesh = dtAllocNavMesh();
			if (!mesh)
			{
				input.close();
				dtFree(mesh);
				return false;
			}
			dtStatus status = mesh->init(&header.params);
			if (dtStatusFailed(status))
			{
				input.close();
				dtFree(mesh);
				return false;
			}

			// Read tiles.
			for (int i = 0; i < header.numTiles; ++i)
			{
				NavMeshTileHeader tileHeader;
				if (!input.read(reinterpret_cast<char*>(&tileHeader), sizeof(NavMeshTileHeader)))
				{
					input.close();
					dtFree(mesh);
					return false;
				}

				if (!tileHeader.tileRef || !tileHeader.dataSize)
					break;

				unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
				if (!data) break;
				memset(data, 0, tileHeader.dataSize);
				if (!input.read(reinterpret_cast<char*>(data), tileHeader.dataSize))
				{
					dtFree(data);
					dtFree(mesh);
					input.close();
					return false;
				}

				mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
			}

			asset = CreateRef<dtNavMesh>();
			asset.reset(mesh);
			return true;
		}

		bool NavMeshImporter::LoadTiledNavMesh(std::ifstream& input, Ref<dtNavMesh>& asset)
		{
			return true;
		}
	}
}
