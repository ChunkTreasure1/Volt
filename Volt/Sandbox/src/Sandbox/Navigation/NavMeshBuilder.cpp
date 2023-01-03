#include "sbpch.h"
#include "NavMeshBuilder.h"
#include "Volt/Log/Log.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"

#include "ext/earcut.h"
#include <array>

namespace Volt
{
	NavMeshBuilder::NavMeshBuilder()
	{
		myBuildSettings.cellSize = 0.3f;
		myBuildSettings.cellHeight = 0.2f;
		myBuildSettings.agentHeight = 2.f;
		myBuildSettings.agentRadius = 0.6f;
		myBuildSettings.agentMaxClimb = 0.9f;
		myBuildSettings.agentMaxSlope = 45.f;
		myBuildSettings.regionMinSize = 8.f;
		myBuildSettings.regionMergeSize = 20.f;
		myBuildSettings.edgeMaxLen = 12.f;
		myBuildSettings.edgeMaxError = 1.3f;
		myBuildSettings.vertsPerPoly = 6.0f;
		myBuildSettings.detailSampleDist = 6.f;
		myBuildSettings.detailSampleMaxError = 1.f;
		myBuildSettings.partitionType = PartitionType::PARTITION_WATERSHED;
	}

	void NavMeshBuilder::ChangeMesh(InputGeom* geom)
	{
		myGeom = geom;

		const BuildSettings* buildSettings = geom->getBuildSettings();
		if (buildSettings)
		{
			myBuildSettings.cellSize = buildSettings->cellSize;
			myBuildSettings.cellHeight = buildSettings->cellHeight;
			myBuildSettings.agentHeight = buildSettings->agentHeight;
			myBuildSettings.agentRadius = buildSettings->agentRadius;
			myBuildSettings.agentMaxClimb = buildSettings->agentMaxClimb;
			myBuildSettings.agentMaxSlope = buildSettings->agentMaxSlope;
			myBuildSettings.regionMinSize = buildSettings->regionMinSize;
			myBuildSettings.regionMergeSize = buildSettings->regionMergeSize;
			myBuildSettings.edgeMaxLen = buildSettings->edgeMaxLen;
			myBuildSettings.edgeMaxError = buildSettings->edgeMaxError;
			myBuildSettings.vertsPerPoly = buildSettings->vertsPerPoly;
			myBuildSettings.detailSampleDist = buildSettings->detailSampleDist;
			myBuildSettings.detailSampleMaxError = buildSettings->detailSampleMaxError;
			myBuildSettings.partitionType = buildSettings->partitionType;
		}
	}

	bool NavMeshBuilder::BuildNavMesh(float unitModifier)
	{
		std::stringstream ss;

		if (!myGeom->getMesh())
		{
			ss << "Build failed, no geometry loaded!";
			VT_CORE_ERROR(ss.str());
			return false;
		}

		//CleanUp();

		const float* bmin = myGeom->getNavMeshBoundsMin();
		const float* bmax = myGeom->getNavMeshBoundsMax();
		const float* verts = myGeom->getMesh()->getVerts();
		const int nverts = myGeom->getMesh()->getVertCount();
		const int* tris = myGeom->getMesh()->getTris();
		const int ntris = myGeom->getMesh()->getTriCount();

		//
		// Step 1. Initialize build config.
		//

		// Init build configuration from GUI
		memset(&myCfg, 0, sizeof(myCfg));
		myCfg.cs = myBuildSettings.cellSize;
		myCfg.ch = myBuildSettings.cellHeight;
		myCfg.walkableSlopeAngle = myBuildSettings.agentMaxSlope;
		myCfg.walkableHeight = (int)ceilf(myBuildSettings.agentHeight / myCfg.ch);
		myCfg.walkableClimb = (int)floorf(myBuildSettings.agentMaxClimb / myCfg.ch);
		myCfg.walkableRadius = (int)ceilf(myBuildSettings.agentRadius / myCfg.cs);
		myCfg.maxEdgeLen = (int)(myBuildSettings.edgeMaxLen / myBuildSettings.cellSize);
		myCfg.maxSimplificationError = myBuildSettings.edgeMaxError;
		myCfg.minRegionArea = (int)rcSqr(myBuildSettings.regionMinSize);		// Note: area = size*size
		myCfg.mergeRegionArea = (int)rcSqr(myBuildSettings.regionMergeSize);	// Note: area = size*size
		myCfg.maxVertsPerPoly = (int)myBuildSettings.vertsPerPoly;
		myCfg.detailSampleDist = myBuildSettings.detailSampleDist < 0.9f ? 0 : myBuildSettings.cellSize * myBuildSettings.detailSampleDist;
		myCfg.detailSampleMaxError = myBuildSettings.cellHeight * myBuildSettings.detailSampleMaxError;

		// Set the area where the navigation will be build.
		// Here the bounds of the input mesh are used, but the
		// area could be specified by an user defined box, etc.
		rcVcopy(myCfg.bmin, bmin);
		rcVcopy(myCfg.bmax, bmax);
		rcCalcGridSize(myCfg.bmin, myCfg.bmax, myCfg.cs, &myCfg.width, &myCfg.height);

		// Reset build times gathering.
		myCtx.resetTimers();

		// Start the build process.	
		myCtx.startTimer(RC_TIMER_TOTAL);

		myCtx.log(RC_LOG_PROGRESS, "Building navigation:");
		myCtx.log(RC_LOG_PROGRESS, " - %d x %d cells", myCfg.width, myCfg.height);
		myCtx.log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

		//
		// Step 2. Rasterize input polygon soup.
		//

		// Allocate voxel heightfield where we rasterize our input data to.
		mySolid = rcAllocHeightfield();
		if (!mySolid)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
			return false;
		}
		if (!rcCreateHeightfield(&myCtx, *mySolid, myCfg.width, myCfg.height, myCfg.bmin, myCfg.bmax, myCfg.cs, myCfg.ch))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
			return false;
		}

		// Allocate array that can hold triangle area types.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		myTriareas = new unsigned char[ntris];
		if (!myTriareas)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
			return false;
		}

		// Find triangles which are walkable based on their slope and rasterize them.
		// If your input data is multiple meshes, you can transform them here, calculate
		// the are type for each of the meshes and rasterize them.
		memset(myTriareas, 0, ntris * sizeof(unsigned char));
		rcMarkWalkableTriangles(&myCtx, myCfg.walkableSlopeAngle, verts, nverts, tris, ntris, myTriareas);
		if (!rcRasterizeTriangles(&myCtx, verts, nverts, tris, myTriareas, ntris, *mySolid, myCfg.walkableClimb))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
			return false;
		}

		if (!myKeepInterResults)
		{
			delete[] myTriareas;
			myTriareas = 0;
		}

		//
		// Step 3. Filter walkables surfaces.
		//

		// Once all geoemtry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		if (myFilterLowHangingObstacles)
			rcFilterLowHangingWalkableObstacles(&myCtx, myCfg.walkableClimb, *mySolid);
		if (myFilterLedgeSpans)
			rcFilterLedgeSpans(&myCtx, myCfg.walkableHeight, myCfg.walkableClimb, *mySolid);
		if (myFilterWalkableLowHeightSpans)
			rcFilterWalkableLowHeightSpans(&myCtx, myCfg.walkableHeight, *mySolid);

		//
		// Step 4. Partition walkable surface to simple regions.
		//

		// Compact the heightfield so that it is faster to handle from now on.
		// This will result more cache coherent data as well as the neighbours
		// between walkable cells will be calculated.
		myChf = rcAllocCompactHeightfield();
		if (!myChf)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
			return false;
		}
		if (!rcBuildCompactHeightfield(&myCtx, myCfg.walkableHeight, myCfg.walkableClimb, *mySolid, *myChf))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
			return false;
		}

		if (!myKeepInterResults)
		{
			rcFreeHeightField(mySolid);
			mySolid = 0;
		}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(&myCtx, myCfg.walkableRadius, *myChf))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
			return false;
		}

		// (Optional) Mark areas.
		const ConvexVolume* vols = myGeom->getConvexVolumes();
		for (int i = 0; i < myGeom->getConvexVolumeCount(); ++i)
			rcMarkConvexPolyArea(&myCtx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *myChf);


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

		if (myBuildSettings.partitionType == PARTITION_WATERSHED)
		{
			// Prepare for region partitioning, by calculating distance field along the walkable surface.
			if (!rcBuildDistanceField(&myCtx, *myChf))
			{
				myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
				return false;
			}

			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildRegions(&myCtx, *myChf, 0, myCfg.minRegionArea, myCfg.mergeRegionArea))
			{
				myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
				return false;
			}
		}
		else if (myBuildSettings.partitionType == PARTITION_MONOTONE)
		{
			// Partition the walkable surface into simple regions without holes.
			// Monotone partitioning does not need distancefield.
			if (!rcBuildRegionsMonotone(&myCtx, *myChf, 0, myCfg.minRegionArea, myCfg.mergeRegionArea))
			{
				myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
				return false;
			}
		}
		else // PARTITION_LAYERS
		{
			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildLayerRegions(&myCtx, *myChf, 0, myCfg.minRegionArea))
			{
				myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
				return false;
			}
		}

		//
		// Step 5. Trace and simplify region contours.
		//

		// Create contours.
		myCset = rcAllocContourSet();
		if (!myCset)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
			return false;
		}
		if (!rcBuildContours(&myCtx, *myChf, myCfg.maxSimplificationError, myCfg.maxEdgeLen, *myCset))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
			return false;
		}

		//
		// Step 6. Build polygons mesh from contours.
		//

		// Build polygon navmesh from the contours.
		myPmesh = rcAllocPolyMesh();
		if (!myPmesh)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
			return false;
		}
		if (!rcBuildPolyMesh(&myCtx, *myCset, myCfg.maxVertsPerPoly, *myPmesh))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
			return false;
		}

		//
		// Step 7. Create detail mesh which allows to access approximate height on each polygon.
		//

		myDmesh = rcAllocPolyMeshDetail();
		if (!myDmesh)
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
			return false;
		}

		if (!rcBuildPolyMeshDetail(&myCtx, *myPmesh, *myChf, myCfg.detailSampleDist, myCfg.detailSampleMaxError, *myDmesh))
		{
			myCtx.log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
			return false;
		}

		if (!myKeepInterResults)
		{
			rcFreeCompactHeightfield(myChf);
			myChf = 0;
			rcFreeContourSet(myCset);
			myCset = 0;
		}

		ConvertToVTNavMesh(unitModifier);

		myCtx.stopTimer(RC_TIMER_TOTAL);

		myCtx.log(RC_LOG_PROGRESS, "Polymesh: %d vertices  %d polygons", myPmesh->nverts, myPmesh->npolys);
		VT_CORE_INFO(myCtx.getLogText(myCtx.getLogCount() - 1));

		myTotalBuildTimeMs = myCtx.getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;
		VT_CORE_INFO(std::format("Navmesh finished building in {0}ms", myTotalBuildTimeMs));

		return true;
	}

	bool NavMeshBuilder::BuildManualNavMesh(Ref<Mesh> geom)
	{
		auto vertices = geom->GetVertices();
		auto indices = geom->GetIndices();
		auto material = AssetManager::GetAsset<Volt::Material>(NavMesh::navmeshMaterialPath);
		Pathfinder::pfNavMesh navmesh;

		myNavMesh = CreateRef<NavMesh>(vertices, indices, material, navmesh);
		RemoveDublicates();

		vertices = myNavMesh->GetVertices();
		indices = myNavMesh->GetIndices();

		std::vector<Builder::Triangle> triangles;

		// Add triangle polygons to navmesh and check edges for portals
		for (const auto& v : vertices)
		{
			navmesh.addVert({ v.position.x, v.position.y, v.position.z });
		}

		for (uint32_t i = 0; i < indices.size(); i += 3)
		{
			Builder::Edge edge1(indices[i], indices[i + 1]);
			Builder::Edge edge2(indices[i + 1], indices[i + 2]);
			Builder::Edge edge3(indices[i + 2], indices[i]);

			Builder::Triangle triangle;
			triangle.edges[0] = edge1;
			triangle.edges[1] = edge2;
			triangle.edges[2] = edge3;
			triangle.index = triangles.size();

			triangles.emplace_back(triangle);
		}

		for (uint32_t i = 0; auto& t : triangles)
		{
			for (uint32_t j = 0; auto& t2 : triangles)
			{
				if (i != j)
				{
					t.SetPortals(t2);
				}
				j++;
			}
			i++;
		}

		for (const auto& t : triangles)
		{
			Pathfinder::pfPoly poly;

			poly.verts = t.GetVerts();

			for (const auto& e : t.edges)
			{
				if (e.portalToTriIndex.has_value())
				{
					Pathfinder::pfLink link;
					link.edge[0] = e.start;
					link.edge[1] = e.end;
					link.polyId = e.portalToTriIndex.value();
					poly.links.emplace_back(link);
				}
			}

			navmesh.addPoly(poly);
		}

		myNavMesh = CreateRef<NavMesh>(vertices, indices, material, navmesh);
		return true;
	}


	void NavMeshBuilder::RemoveDublicates()
	{
		auto vertices = myNavMesh->GetVertices();
		auto indices = myNavMesh->GetIndices();
		auto material = myNavMesh->GetMaterial();

		std::unordered_map<uint32_t, uint32_t> indexMap;

		for (uint32_t i = 0; i < vertices.size(); ++i)
		{
			for (uint32_t j = 0; j < vertices.size(); ++j)
			{
				if (i != j && gem::distance(vertices[i].position, vertices[j].position) < 0.1f && indexMap.find(i) == indexMap.end())
				{
					indexMap[j] = i;
				}
			}
		}

		for (const auto& i : indexMap)
		{
			std::replace(indices.begin(), indices.end(), i.first, i.second);
		}

		myNavMesh = CreateRef<Volt::NavMesh>(vertices, indices, material, myNavMesh->GetNavMeshData());
	}

	void NavMeshBuilder::ConvertToVTNavMesh(float unitModifier)
	{
		if (!myPmesh->nverts)
		{
			return;
		}

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		Ref<Material> material = AssetManager::GetAsset<Volt::Material>(NavMesh::navmeshMaterialPath);

		Pathfinder::pfNavMesh navmesh;

		const int nvp = myPmesh->nvp;
		const float cs = myPmesh->cs;
		const float ch = myPmesh->ch;
		const float* orig = myPmesh->bmin;
		
		for (int i = 0; i < myPmesh->nverts; i++)
		{
			// Convert to world space.
			const unsigned short* v = &myPmesh->verts[i * 3];
			const float x = orig[0] + v[0] * cs;
			const float y = orig[1] + v[1] * ch;
			const float z = orig[2] + v[2] * cs;

			Vertex vert;
			vert.position = gem::vec3(x * unitModifier, y * unitModifier, z * unitModifier);
			vertices.emplace_back(vert);
			navmesh.addVert({ vert.position.x, vert.position.y, vert.position.z });
		}

		for (int i = 0; i < myPmesh->npolys; ++i)
		{
			const unsigned short* p = &myPmesh->polys[i * nvp * 2];
			const unsigned int* meshDef = &myDmesh->meshes[i * 4];
			const unsigned int baseVerts = meshDef[0];

			Pathfinder::pfPoly poly;

			// Iterate the vertices.
			for (int j = 0; j < nvp; ++j)
			{
				if (p[j] == RC_MESH_NULL_IDX)
					break; // End of vertices.

				int nextIndex = (j + 1 == nvp || p[j + 1] == RC_MESH_NULL_IDX) ? 0 : j + 1;

				if (p[j + nvp] != RC_MESH_NULL_IDX)
				{
					auto first = vertices[p[j]].position;
					auto second = vertices[p[nextIndex]].position;

					Pathfinder::pfLink link;
					link.edge[0] = p[j];
					link.edge[1] = p[nextIndex];
					link.polyId = p[j + nvp];

					poly.links.emplace_back(link);
				}

				// Do something with the vertices.
				poly.verts.emplace_back(p[j]);
			}

			// The number type to use for tessellation
			using Coord = double;

			// The index type. Defaults to uint32_t, but you can also pass uint16_t if you know that your
			// data won't have more than 65536 vertices.
			using N = uint32_t;

			// Create array
			using Point = std::array<Coord, 2>;

			std::vector<std::vector<Point>> ec_polygon;

			std::vector<Point> points;
			for (const auto& v : poly.verts)
			{
				Point point = { vertices[v].position.x, vertices[v].position.z };
				points.emplace_back(point);
			}

			ec_polygon.emplace_back(points);
		
			auto earcutIndices = mapbox::earcut<N>(ec_polygon);

			std::vector<uint32_t> inds;
			for (const auto& ind : earcutIndices)
			{
				inds.emplace_back(poly.verts[ind]);
			}

			std::reverse(inds.begin(), inds.end());
			indices.insert(indices.end(), inds.begin(), inds.end());

			navmesh.addPoly(poly);
		}
		myNavMesh = CreateRef<NavMesh>(vertices, indices, material, navmesh);
	}

	void NavMeshBuilder::CleanUp()
	{
		delete[] myTriareas;
		myTriareas = 0;
		rcFreeHeightField(mySolid);
		mySolid = 0;
		rcFreeCompactHeightfield(myChf);
		myChf = 0;
		rcFreeContourSet(myCset);
		myCset = 0;
		rcFreePolyMesh(myPmesh);
		myPmesh = 0;
		rcFreePolyMeshDetail(myDmesh);
		myDmesh = 0;
	}
}

