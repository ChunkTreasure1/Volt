#pragma once
#include "BuildInterfaces.h"
#include "Navigation/NavMesh/VTNavMesh.h"
#include "InputGeom.h"

#include <DetourNavMesh.h>
#include <Recast.h>

#include <glm/glm.hpp>

enum PartitionType
{
	PARTITION_WATERSHED,
	PARTITION_MONOTONE,
	PARTITION_LAYERS,
};

struct LinearAllocator;
struct FastLZCompressor;
struct MeshProcess;
struct TileCacheData;

class RecastBuilder
{
public:
	RecastBuilder(RecastBuildSettings& buildSettings);
	~RecastBuilder() = default;

	void SetInputGeom(Ref<Volt::Mesh> mesh) { m_geom = CreateRef<InputGeom>(mesh); };
	void SetInputGeom(Ref<InputGeom> geom) { m_geom = geom; };

	Ref<Volt::AI::NavMesh> BuildSingleNavMesh();
	Ref<Volt::AI::NavMesh> BuildTiledNavMesh();

	RecastBuildContext* GetContext() { return m_ctx.get(); }

	void AddNavLinkConnection(Volt::AI::NavLinkConnection link);
	void RemoveNavLinkConnection(uint32_t index);
	void ClearNavLinkConnections();

private:
	// General Recast
	void CleanUp();

	// TileCache
	int rasterizeTileLayers(const int tx, const int ty, const rcConfig& cfg, struct TileCacheData* tiles, const int maxTiles);

	// General Recast
	rcConfig m_cfg;
	Ref<InputGeom> m_geom;
	Ref<RecastBuildContext> m_ctx;
	RecastBuildSettings* m_buildSettings;

	bool m_keepInterResults = false;
	float m_totalBuildTimeMs = 0.f;

	bool m_filterLowHangingObstacles = true;
	bool m_filterLedgeSpans = true;
	bool m_filterWalkableLowHeightSpans = true;

	// OffMesh
	Vector<Volt::AI::NavLinkConnection> m_navLinkConnections;

	// SoloMesh
	unsigned char* m_triareas;
	rcHeightfield* m_solid;
	rcCompactHeightfield* m_chf;
	rcContourSet* m_cset;
	rcPolyMesh* m_pmesh;
	rcPolyMeshDetail* m_dmesh;

	// TileCache
	Ref<LinearAllocator> m_talloc;
	Ref<FastLZCompressor> m_tcomp;
	Ref<MeshProcess> m_tmproc;

	float m_cacheBuildTimeMs;
	int m_cacheCompressedSize;
	int m_cacheRawSize;
	int m_cacheLayerCount;
	unsigned int m_cacheBuildMemUsage;
};
