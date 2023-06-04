#include "nvpch.h"
#include "RecastBuilder.h"

#include "Navigation/Core/CoreInterfaces.h"
#include "NavigationEditor/Tools/NavMeshDebugDrawer.h"

#include <Volt/Log/Log.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Asset/Mesh/Material.h>

#include <DetourAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourDebugDraw.h>
#include <DetourCommon.h>
#include <DetourTileCache.h>
#include <fastlz.h>

#include <array>
#include <unordered_set>

static const int MAX_LAYERS = 32;
static const int EXPECTED_LAYERS_PER_TILE = 4;

struct TileCacheData
{
	unsigned char* data;
	int dataSize;
};

struct RasterizationContext
{
	RasterizationContext() :
		solid(0),
		triareas(0),
		lset(0),
		chf(0),
		ntiles(0)
	{
		memset(tiles, 0, sizeof(TileCacheData) * MAX_LAYERS);
	}

	~RasterizationContext()
	{
		rcFreeHeightField(solid);
		delete[] triareas;
		rcFreeHeightfieldLayerSet(lset);
		rcFreeCompactHeightfield(chf);
		for (int i = 0; i < MAX_LAYERS; ++i)
		{
			dtFree(tiles[i].data);
			tiles[i].data = 0;
		}
	}

	rcHeightfield* solid;
	unsigned char* triareas;
	rcHeightfieldLayerSet* lset;
	rcCompactHeightfield* chf;
	TileCacheData tiles[MAX_LAYERS];
	int ntiles;
};

static int calcLayerBufferSize(const int gridWidth, const int gridHeight)
{
	const int headerSize = dtAlign4(sizeof(dtTileCacheLayerHeader));
	const int gridSize = gridWidth * gridHeight;
	return headerSize + gridSize * 4;
}

struct FastLZCompressor : public dtTileCacheCompressor
{
	virtual int maxCompressedSize(const int bufferSize)
	{
		return (int)(bufferSize * 1.05f);
	}

	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
							  unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
	{
		*compressedSize = fastlz_compress((const void* const)buffer, bufferSize, compressed);
		return DT_SUCCESS;
	}

	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
								unsigned char* buffer, const int maxBufferSize, int* bufferSize)
	{
		*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
		return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
	}
};

struct LinearAllocator : public dtTileCacheAlloc
{
	unsigned char* buffer;
	size_t capacity;
	size_t top;
	size_t high;

	LinearAllocator(const size_t cap) : buffer(0), capacity(0), top(0), high(0)
	{
		resize(cap);
	}

	~LinearAllocator()
	{
		dtFree(buffer);
	}

	void resize(const size_t cap)
	{
		if (buffer) dtFree(buffer);
		buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
		capacity = cap;
	}

	virtual void reset()
	{
		high = dtMax(high, top);
		top = 0;
	}

	virtual void* alloc(const size_t size)
	{
		if (!buffer)
			return 0;
		if (top + size > capacity)
			return 0;
		unsigned char* mem = &buffer[top];
		top += size;
		return mem;
	}

	virtual void free(void* /*ptr*/)
	{
		// Empty
	}
};

struct MeshProcess : public dtTileCacheMeshProcess
{
	InputGeom* m_geom;

	inline MeshProcess() : m_geom(0)
	{
	}

	inline void init(InputGeom* geom)
	{
		m_geom = geom;
	}

	virtual void process(struct dtNavMeshCreateParams* params,
						 unsigned char* polyAreas, unsigned short* polyFlags)
	{
		// Update poly flags from areas.
		for (int i = 0; i < params->polyCount; ++i)
		{
			if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
				polyAreas[i] = Volt::AI::POLYAREA_GROUND;

			if (polyAreas[i] == Volt::AI::POLYAREA_GROUND ||
				polyAreas[i] == Volt::AI::POLYAREA_GRASS ||
				polyAreas[i] == Volt::AI::POLYAREA_ROAD)
			{
				polyFlags[i] = Volt::AI::POLYFLAGS_WALK;
			}
			else if (polyAreas[i] == Volt::AI::POLYAREA_WATER)
			{
				polyFlags[i] = Volt::AI::POLYFLAGS_SWIM;
			}
			else if (polyAreas[i] == Volt::AI::POLYAREA_DOOR)
			{
				polyFlags[i] = Volt::AI::POLYFLAGS_WALK | Volt::AI::POLYFLAGS_DOOR;
			}
		}

		// Pass in off-mesh connections.
		if (m_geom)
		{
			params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params->offMeshConRad = m_geom->getOffMeshConnectionRads();
			params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params->offMeshConUserID = m_geom->getOffMeshConnectionId();
			params->offMeshConCount = m_geom->getOffMeshConnectionCount();
		}
	}
};

int RecastBuilder::rasterizeTileLayers(
							   const int tx, const int ty,
							   const rcConfig& cfg,
							   TileCacheData* tiles,
							   const int maxTiles)
{
	if (!m_geom || !m_geom->getChunkyMesh())
	{
		VT_CORE_ERROR("buildTile: Input mesh is not specified.");
		return 0;
	}

	FastLZCompressor comp;
	RasterizationContext rc;

	const float* verts = m_geom->getVerts();
	const int nverts = m_geom->getVertCount();
	const rcChunkyTriMesh* chunkyMesh = m_geom->getChunkyMesh();

	// Tile bounds.
	const float tcs = cfg.tileSize * cfg.cs;

	rcConfig tcfg;
	memcpy(&tcfg, &cfg, sizeof(tcfg));

	tcfg.bmin[0] = cfg.bmin[0] + tx * tcs;
	tcfg.bmin[1] = cfg.bmin[1];
	tcfg.bmin[2] = cfg.bmin[2] + ty * tcs;
	tcfg.bmax[0] = cfg.bmin[0] + (tx + 1) * tcs;
	tcfg.bmax[1] = cfg.bmax[1];
	tcfg.bmax[2] = cfg.bmin[2] + (ty + 1) * tcs;
	tcfg.bmin[0] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmin[2] -= tcfg.borderSize * tcfg.cs;
	tcfg.bmax[0] += tcfg.borderSize * tcfg.cs;
	tcfg.bmax[2] += tcfg.borderSize * tcfg.cs;

	// Allocate voxel heightfield where we rasterize our input data to.
	rc.solid = rcAllocHeightfield();
	if (!rc.solid)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(m_ctx.get(), *rc.solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
	{
		VT_CORE_ERROR("buildNavigation: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	rc.triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
	if (!rc.triareas)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'm_triareas' ({0}).", chunkyMesh->maxTrisPerChunk);
		return 0;
	}

	float tbmin[2], tbmax[2];
	tbmin[0] = tcfg.bmin[0];
	tbmin[1] = tcfg.bmin[2];
	tbmax[0] = tcfg.bmax[0];
	tbmax[1] = tcfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
	{
		return 0; // empty
	}

	for (int i = 0; i < ncid; ++i)
	{
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* tris = &chunkyMesh->tris[node.i * 3];
		const int ntris = node.n;

		memset(rc.triareas, 0, ntris * sizeof(unsigned char));
		rcMarkWalkableTriangles(m_ctx.get(), tcfg.walkableSlopeAngle,
								verts, nverts, tris, ntris, rc.triareas);

		if (!rcRasterizeTriangles(m_ctx.get(), verts, nverts, tris, rc.triareas, ntris, *rc.solid, tcfg.walkableClimb))
			return 0;
	}

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(m_ctx.get(), tcfg.walkableClimb, *rc.solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(m_ctx.get(), tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(m_ctx.get(), tcfg.walkableHeight, *rc.solid);


	rc.chf = rcAllocCompactHeightfield();
	if (!rc.chf)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(m_ctx.get(), tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid, *rc.chf))
	{
		VT_CORE_ERROR("buildNavigation: Could not build compact data.");
		return 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx.get(), tcfg.walkableRadius, *rc.chf))
	{
		VT_CORE_ERROR("buildNavigation: Could not erode.");
		return 0;
	}

	// (Optional) Mark areas.
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	{
		rcMarkConvexPolyArea(m_ctx.get(), vols[i].verts, vols[i].nverts,
							 vols[i].hmin, vols[i].hmax,
							 (unsigned char)vols[i].area, *rc.chf);
	}

	rc.lset = rcAllocHeightfieldLayerSet();
	if (!rc.lset)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'lset'.");
		return 0;
	}
	if (!rcBuildHeightfieldLayers(m_ctx.get(), *rc.chf, tcfg.borderSize, tcfg.walkableHeight, *rc.lset))
	{
		VT_CORE_ERROR("buildNavigation: Could not build heighfield layers.");
		return 0;
	}

	rc.ntiles = 0;
	for (int i = 0; i < rcMin(rc.lset->nlayers, MAX_LAYERS); ++i)
	{
		TileCacheData* tile = &rc.tiles[rc.ntiles++];
		const rcHeightfieldLayer* layer = &rc.lset->layers[i];

		// Store header
		dtTileCacheLayerHeader header;
		header.magic = DT_TILECACHE_MAGIC;
		header.version = DT_TILECACHE_VERSION;

		// Tile layer location in the navmesh.
		header.tx = tx;
		header.ty = ty;
		header.tlayer = i;
		dtVcopy(header.bmin, layer->bmin);
		dtVcopy(header.bmax, layer->bmax);

		// Tile info.
		header.width = (unsigned char)layer->width;
		header.height = (unsigned char)layer->height;
		header.minx = (unsigned char)layer->minx;
		header.maxx = (unsigned char)layer->maxx;
		header.miny = (unsigned char)layer->miny;
		header.maxy = (unsigned char)layer->maxy;
		header.hmin = (unsigned short)layer->hmin;
		header.hmax = (unsigned short)layer->hmax;

		dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
												&tile->data, &tile->dataSize);
		if (dtStatusFailed(status))
		{
			return 0;
		}
	}

	// Transfer ownsership of tile data from build context to the caller.
	int n = 0;
	for (int i = 0; i < rcMin(rc.ntiles, maxTiles); ++i)
	{
		tiles[n++] = rc.tiles[i];
		rc.tiles[i].data = 0;
		rc.tiles[i].dataSize = 0;
	}

	return n;
}

RecastBuilder::RecastBuilder(RecastBuildSettings& buildSettings) :
	m_keepInterResults(true),
	m_totalBuildTimeMs(0),
	m_triareas(0),
	m_solid(0),
	m_chf(0),
	m_cset(0),
	m_pmesh(0),
	m_dmesh(0)
{
	m_buildSettings = &buildSettings;

	m_ctx = CreateRef<RecastBuildContext>();

	m_talloc = CreateRef<LinearAllocator>(32000);
	m_tcomp = CreateRef<FastLZCompressor>();
	m_tmproc = CreateRef<MeshProcess>();
}

Ref<Volt::AI::NavMesh> RecastBuilder::BuildSingleNavMesh()
{
	if (!m_geom || !m_geom->getVertCount())
	{
		VT_CORE_ERROR("buildNavigation: Input mesh is not specified.");
		return nullptr;
	}

	CleanUp();

	dtStatus status;
	Ref<Volt::AI::NavMesh> result;

	gem::vec3 bmin(0.f);
	gem::vec3 bmax(0.f);

	const float* verts = m_geom->getVerts();
	const int nverts = m_geom->getVertCount();
	const int* tris = m_geom->getTris();
	const int ntris = m_geom->getTriCount();

	rcCalcBounds(verts, nverts, (float*)&bmin, (float*)&bmax);

	//
	// Step 1. Initialize build config.
	//

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_buildSettings->cellSize;
	m_cfg.ch = m_buildSettings->cellHeight;
	m_cfg.walkableSlopeAngle = m_buildSettings->agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_buildSettings->agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_buildSettings->agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_buildSettings->agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_buildSettings->edgeMaxLen / m_buildSettings->cellSize);
	m_cfg.maxSimplificationError = m_buildSettings->edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_buildSettings->regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_buildSettings->regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_buildSettings->vertsPerPoly;
	m_cfg.detailSampleDist = m_buildSettings->detailSampleDist < 0.9f ? 0 : m_buildSettings->cellSize * m_buildSettings->detailSampleDist;
	m_cfg.detailSampleMaxError = m_buildSettings->cellHeight * m_buildSettings->detailSampleMaxError;

	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(m_cfg.bmin, (const float*)&bmin);
	rcVcopy(m_cfg.bmax, (const float*)&bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// Reset build times gathering.
	m_ctx->resetTimers();

	// Start the build process.	
	m_ctx->startTimer(RC_TIMER_TOTAL);

	VT_CORE_INFO("Building navigation:");
	VT_CORE_INFO(" - {0} x {1} cells", m_cfg.width, m_cfg.height);
	VT_CORE_INFO(" - {0}K verts, {1}K tris", nverts / 1000.0f, ntris / 1000.0f);

	//
	// Step 2. Rasterize input polygon soup.
	//

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'solid'.");
		return nullptr;
	}
	if (!rcCreateHeightfield(m_ctx.get(), *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		VT_CORE_ERROR("buildNavigation: Could not create solid heightfield.");
		return nullptr;
	}

	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[ntris];
	if (!m_triareas)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'm_triareas' ({0}).", ntris);
		return nullptr;
	}

	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, ntris * sizeof(unsigned char));
	rcMarkWalkableTriangles(m_ctx.get(), m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	if (!rcRasterizeTriangles(m_ctx.get(), verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb))
	{
		VT_CORE_ERROR("buildNavigation: Could not rasterize triangles.");
		return nullptr;
	}

	if (!m_keepInterResults)
	{
		delete[] m_triareas;
		m_triareas = 0;
	}

	//
	// Step 3. Filter walkables surfaces.
	//

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(m_ctx.get(), m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(m_ctx.get(), m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(m_ctx.get(), m_cfg.walkableHeight, *m_solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'chf'.");
		return nullptr;
	}
	if (!rcBuildCompactHeightfield(m_ctx.get(), m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		VT_CORE_ERROR("buildNavigation: Could not build compact data.");
		return nullptr;
	}

	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx.get(), m_cfg.walkableRadius, *m_chf))
	{
		VT_CORE_ERROR("buildNavigation: Could not erode.");
		return nullptr;
	}

	//// (Optional) Mark areas.
	//const ConvexVolume* vols = m_geom->getConvexVolumes();
	//for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	//	rcMarkConvexPolyArea(m_ctx.get(), vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	if (m_buildSettings->partitionType == PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(m_ctx.get(), *m_chf))
		{
			VT_CORE_ERROR("buildNavigation: Could not build distance field.");
			return nullptr;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(m_ctx.get(), *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			VT_CORE_ERROR("buildNavigation: Could not build watershed regions.");
			return nullptr;
		}
	}
	else if (m_buildSettings->partitionType == PARTITION_MONOTONE)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(m_ctx.get(), *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			VT_CORE_ERROR("buildNavigation: Could not build monotone regions.");
			return nullptr;
		}
	}
	else if (m_buildSettings->partitionType == PARTITION_LAYERS)
	{
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildLayerRegions(m_ctx.get(), *m_chf, 0, m_cfg.minRegionArea))
		{
			VT_CORE_ERROR("buildNavigation: Could not build layer regions.");
			return nullptr;
		}
	}

	//
	// Step 5. Trace and simplify region contours.
	//

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'cset'.");
		return nullptr;
	}
	if (!rcBuildContours(m_ctx.get(), *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		VT_CORE_ERROR("buildNavigation: Could not create contours.");
		return nullptr;
	}

	//
	// Step 6. Build polygons mesh from contours.
	//

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'pmesh'.");
		return nullptr;
	}
	if (!rcBuildPolyMesh(m_ctx.get(), *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		VT_CORE_ERROR("buildNavigation: Could not triangulate contours.");
		return nullptr;
	}

	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//

	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		VT_CORE_ERROR("buildNavigation: Out of memory 'pmdtl'.");
		return nullptr;
	}

	if (!rcBuildPolyMeshDetail(m_ctx.get(), *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		VT_CORE_ERROR("buildNavigation: Could not build detail mesh.");
		return nullptr;
	}

	if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}

	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//

	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
				m_pmesh->areas[i] = Volt::AI::POLYAREA_GROUND;

			if (m_pmesh->areas[i] == Volt::AI::POLYAREA_GROUND ||
				m_pmesh->areas[i] == Volt::AI::POLYAREA_GRASS ||
				m_pmesh->areas[i] == Volt::AI::POLYAREA_ROAD)
			{
				m_pmesh->flags[i] = Volt::AI::POLYFLAGS_WALK;
			}
			else if (m_pmesh->areas[i] == Volt::AI::POLYAREA_WATER)
			{
				m_pmesh->flags[i] = Volt::AI::POLYFLAGS_SWIM;
			}
			else if (m_pmesh->areas[i] == Volt::AI::POLYAREA_DOOR)
			{
				m_pmesh->flags[i] = Volt::AI::POLYFLAGS_WALK | Volt::AI::POLYFLAGS_DOOR;
			}
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;		
		params.walkableHeight = m_buildSettings->agentHeight;
		params.walkableRadius = m_buildSettings->agentRadius;
		params.walkableClimb = m_buildSettings->agentMaxClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		// NavLink
		{
			params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params.offMeshConRad = m_geom->getOffMeshConnectionRads();
			params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params.offMeshConUserID = m_geom->getOffMeshConnectionId();
			params.offMeshConCount = m_geom->getOffMeshConnectionCount();
		}

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			VT_CORE_ERROR("Could not build Detour navmesh.");
			return nullptr;
		}

		auto navMesh = CreateRef<dtNavMesh>();

		if (!navMesh)
		{
			dtFree(navData);
			VT_CORE_ERROR("Could not create Detour navmesh");
			return nullptr;
		}

		status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			VT_CORE_ERROR("Could not init Detour navmesh");
			return nullptr;
		}

		result = CreateRef<Volt::AI::NavMesh>(navMesh);

		if (result)
		{
			result->GetCrowd()->SetMaxAgents(m_buildSettings->maxAgents);
			result->GetCrowd()->SetMaxAgentRadius(m_buildSettings->agentRadius);
		}
	}

	m_ctx->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	VT_CORE_INFO(">> Detailed Polymesh: {0} vertices, {1} triangles", m_dmesh->nverts, m_dmesh->ntris);
	VT_CORE_INFO(">> Polymesh: {0} vertices, {1} polygons", m_pmesh->nverts, m_pmesh->npolys);

	m_totalBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
	VT_CORE_INFO(">> NavMesh finished building in {0}ms", m_totalBuildTimeMs);

	return result;
}

Ref<Volt::AI::NavMesh> RecastBuilder::BuildTiledNavMesh()
{
	if (!m_geom || !m_geom->getVertCount())
	{
		VT_CORE_ERROR("buildTiledNavigation: Input mesh is not specified.");
		return nullptr;
	}

	CleanUp();

	m_tmproc->init(m_geom.get());

	dtStatus status;
	Ref<Volt::AI::NavMesh> result;

	gem::vec3 bmin(0.f);
	gem::vec3 bmax(0.f);

	const float* verts = m_geom->getVerts();
	const int nverts = m_geom->getVertCount();

	rcCalcBounds(verts, nverts, (float*)&bmin, (float*)&bmax);

	// Init cache
	int gw = 0, gh = 0;
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &gw, &gh);
	const int ts = (int)m_buildSettings->tileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;

	// Generation params.
	rcConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = m_buildSettings->cellSize;
	cfg.ch = m_buildSettings->cellHeight;
	cfg.walkableSlopeAngle = m_buildSettings->agentMaxSlope;
	cfg.walkableHeight = (int)ceilf(m_buildSettings->agentHeight / cfg.ch);
	cfg.walkableClimb = (int)floorf(m_buildSettings->agentMaxClimb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(m_buildSettings->agentRadius / cfg.cs);
	cfg.maxEdgeLen = (int)(m_buildSettings->edgeMaxLen / m_buildSettings->cellSize);
	cfg.maxSimplificationError = m_buildSettings->edgeMaxError;
	cfg.minRegionArea = (int)rcSqr(m_buildSettings->regionMinSize);		// Note: area = size*size
	cfg.mergeRegionArea = (int)rcSqr(m_buildSettings->regionMergeSize);	// Note: area = size*size
	cfg.maxVertsPerPoly = (int)m_buildSettings->vertsPerPoly;
	cfg.tileSize = (int)m_buildSettings->tileSize;
	cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
	cfg.width = cfg.tileSize + cfg.borderSize * 2;
	cfg.height = cfg.tileSize + cfg.borderSize * 2;
	cfg.detailSampleDist = m_buildSettings->detailSampleDist < 0.9f ? 0 : m_buildSettings->cellSize * m_buildSettings->detailSampleDist;
	cfg.detailSampleMaxError = m_buildSettings->cellHeight * m_buildSettings->detailSampleMaxError;
	rcVcopy(m_cfg.bmin, (const float*)&bmin);
	rcVcopy(m_cfg.bmax, (const float*)&bmax);

	// Tile cache params.
	dtTileCacheParams tcparams;
	memset(&tcparams, 0, sizeof(tcparams));
	rcVcopy(tcparams.orig, (const float*)&bmin);
	tcparams.cs = m_buildSettings->cellSize;
	tcparams.ch = m_buildSettings->cellHeight;
	tcparams.width = (int)m_buildSettings->tileSize;
	tcparams.height = (int)m_buildSettings->tileSize;
	tcparams.walkableHeight = m_buildSettings->agentHeight;
	tcparams.walkableRadius = m_buildSettings->agentRadius;
	tcparams.walkableClimb = m_buildSettings->agentMaxClimb;
	tcparams.maxSimplificationError = m_buildSettings->edgeMaxError;
	tcparams.maxTiles = tw * th * EXPECTED_LAYERS_PER_TILE;
	tcparams.maxObstacles = 128;

	auto tileCache = CreateRef<dtTileCache>();
	if (!tileCache)
	{
		VT_CORE_ERROR("buildTiledNavigation: Could not allocate tile cache.");
		return nullptr;
	}

	status = tileCache->init(&tcparams, m_talloc.get(), m_tcomp.get(), m_tmproc.get());
	if (dtStatusFailed(status))
	{
		VT_CORE_ERROR("buildTiledNavigation: Could not init tile cache.");
		return nullptr;
	}

	auto navMesh = CreateRef<dtNavMesh>();
	if (!navMesh)
	{
		VT_CORE_ERROR("buildTiledNavigation: Could not allocate navmesh.");
		return nullptr;
	}

	int tileBits = rcMin((int)dtIlog2(dtNextPow2(tw * th * EXPECTED_LAYERS_PER_TILE)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;
	auto maxTiles = 1 << tileBits;
	auto maxPolysPerTile = 1 << polyBits;

	dtNavMeshParams params;
	memset(&params, 0, sizeof(params));
	rcVcopy(params.orig, (const float*)&bmin);
	params.tileWidth = m_buildSettings->tileSize * m_buildSettings->cellSize;
	params.tileHeight = m_buildSettings->tileSize * m_buildSettings->cellSize;
	params.maxTiles = maxTiles;
	params.maxPolys = maxPolysPerTile;

	status = navMesh->init(&params);
	if (dtStatusFailed(status))
	{
		VT_CORE_ERROR("buildTiledNavigation: Could not init navmesh.");
		return nullptr;
	}

	// Preprocess tiles.

	m_ctx->resetTimers();

	m_cacheLayerCount = 0;
	m_cacheCompressedSize = 0;
	m_cacheRawSize = 0;

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			int ntiles = rasterizeTileLayers(x, y, cfg, tiles, MAX_LAYERS);

			for (int i = 0; i < ntiles; ++i)
			{
				TileCacheData* tile = &tiles[i];
				status = tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
				if (dtStatusFailed(status))
				{
					dtFree(tile->data);
					tile->data = 0;
					continue;
				}

				m_cacheLayerCount++;
				m_cacheCompressedSize += tile->dataSize;
				m_cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
			}
		}
	}

	// Build initial meshes
	m_ctx->startTimer(RC_TIMER_TOTAL);
	for (int y = 0; y < th; ++y)
		for (int x = 0; x < tw; ++x)
			tileCache->buildNavMeshTilesAt(x, y, navMesh.get());
	m_ctx->stopTimer(RC_TIMER_TOTAL);

	m_cacheBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
	m_cacheBuildMemUsage = static_cast<unsigned int>(m_talloc->high);

	const dtNavMesh* nav = navMesh.get();
	int navmeshMemUsage = 0;
	for (int i = 0; i < nav->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = nav->getTile(i);
		if (tile->header)
		{
			navmeshMemUsage += tile->dataSize;
		}
	}

	result = CreateRef<Volt::AI::NavMesh>(navMesh, tileCache);

	auto gridSize = tw * th;
	const float compressionRatio = (float)m_cacheCompressedSize / (float)(m_cacheRawSize + 1);

	// Show performance stats.
	VT_CORE_INFO(">> Layers: {0}", m_cacheLayerCount);
	VT_CORE_INFO(">> Layers (per tile) {0}", (float)m_cacheLayerCount / (float)gridSize);
	VT_CORE_INFO(">> Memory: {0} kB / {1} kB ({2})", m_cacheCompressedSize / 1024.0f, m_cacheRawSize / 1024.0f, compressionRatio * 100.0f);
	VT_CORE_INFO(">> Build Peak Mem Usage: {0} kB", m_cacheBuildMemUsage / 1024.0f);
	VT_CORE_INFO(">> NavMesh finished building in {0}ms", m_cacheBuildTimeMs);

	return result;
}

void RecastBuilder::AddNavLinkConnection(Volt::AI::NavLinkConnection link)
{
	if (m_geom)
	{
		m_navLinkConnections.emplace_back(link);
		m_geom->addOffMeshConnection((const float*)&link.start, (const float*)&link.end, 0.2f, link.bidirectional, Volt::AI::PolyAreas::POLYAREA_GROUND, (link.active) ? Volt::AI::PolyFlags::POLYFLAGS_WALK : Volt::AI::PolyFlags::POLYFLAGS_DISABLED);
	}
}

void RecastBuilder::RemoveNavLinkConnection(uint32_t index)
{
	if (m_geom)
	{
		m_navLinkConnections.erase(m_navLinkConnections.begin() + index);
		m_geom->deleteOffMeshConnection(index);
	}
}

void RecastBuilder::ClearNavLinkConnections()
{
	if (m_geom)
	{
		m_navLinkConnections.clear();
		m_geom->clearOffMeshConnections();
	}
}

void RecastBuilder::CleanUp()
{
	delete[] m_triareas;
	m_triareas = 0;
	rcFreeHeightField(m_solid);
	m_solid = 0;
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;
	rcFreePolyMesh(m_pmesh);
	m_pmesh = 0;
	rcFreePolyMeshDetail(m_dmesh);
	m_dmesh = 0;
}

