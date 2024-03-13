#include "nvpch.h"
#include "NavMeshImporter.h"

#include <Volt/Log/Log.h>
#include <Volt/Core/Profiling.h>

#include <Volt/Asset/AssetManager.h>

#include <Volt/Utility/FileIO/BinaryStreamWriter.h>
#include <Volt/Utility/FileIO/BinaryStreamReader.h>

namespace Volt
{
	namespace AI
	{
		static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
		static const int NAVMESHSET_VERSION = 2;

		static const int TILECACHESET_MAGIC = 'T' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'TSET';
		static const int TILECACHESET_VERSION = 2;

		bool NavMeshImporter::SaveNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset)
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

		bool NavMeshImporter::LoadNavMesh(BinaryStreamReader& input, Ref<dtNavMesh>& asset)
		{
			// Read header.
			NavMeshSetHeader header;
			input.Read(header);

			switch (header.magic)
			{
				case NAVMESHSET_MAGIC:
				{
					return LoadSingleNavMesh(input, header, asset);
				}
				case TILECACHESET_MAGIC:
				{
					return LoadTiledNavMesh(input, header, asset);
				}
				default:
				{
					return false;
				}
			}
		}

		bool NavMeshImporter::LoadNavMeshLegacy(std::ifstream& input, Ref<dtNavMesh>& asset)
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
					return LoadSingleNavMeshLegacy(input, asset);
				}
				case TILECACHESET_MAGIC:
				{
					//return LoadTiledNavMesh(input, asset);
				}
				default:
				{
					input.close();
					return false;
				}
			}
		}

		bool NavMeshImporter::SaveSingleNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset)
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
			output.Write(header);

			// Store tiles.
			for (int i = 0; i < mesh->getMaxTiles(); ++i)
			{
				const dtMeshTile* tile = mesh->getTile(i);
				if (!tile || !tile->header || !tile->dataSize) continue;

				NavMeshTileHeader tileHeader;
				tileHeader.tileRef = mesh->getTileRef(tile);
				tileHeader.dataSize = tile->dataSize;
				output.Write(tileHeader);
				output.Write(tile->data);
			}
			return true;
		}

		bool NavMeshImporter::SaveTiledNavMesh(BinaryStreamWriter& output, Ref<NavMesh>& asset)
		{
			return true;
		}

		bool NavMeshImporter::LoadSingleNavMesh(BinaryStreamReader& input, const NavMeshSetHeader& header, Ref<dtNavMesh>& asset)
		{
			if (header.magic != NAVMESHSET_MAGIC)
			{
				return false;
			}
			if (header.version != NAVMESHSET_VERSION)
			{
				return false;
			}

			dtNavMesh* mesh = dtAllocNavMesh();
			if (!mesh)
			{
				dtFree(mesh);
				return false;
			}
			dtStatus status = mesh->init(&header.params);
			if (dtStatusFailed(status))
			{
				dtFree(mesh);
				return false;
			}

			// Read tiles.
			for (int i = 0; i < header.numTiles; ++i)
			{
				NavMeshTileHeader tileHeader;
				input.Read(tileHeader);

				if (!tileHeader.tileRef || !tileHeader.dataSize)
					break;

				unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
				if (!data) break;
				memset(data, 0, tileHeader.dataSize);

				input.Read(data);
				mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
			}

			asset = CreateRef<dtNavMesh>();
			asset.reset(mesh);
			return true;
		}

		bool NavMeshImporter::LoadTiledNavMesh(BinaryStreamReader& input, const NavMeshSetHeader& header, Ref<dtNavMesh>& asset)
		{
			return true;
		}

		bool NavMeshImporter::LoadSingleNavMeshLegacy(std::ifstream& input, Ref<dtNavMesh>& asset)
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
	}
}
