#pragma once
#include "ext/Recast/Recast.h"
#include "ext/InputGeom.h"
#include "ext/SampleInterfaces.h"
#include "Volt/AI/NavMesh.h"

namespace Volt
{
	enum PartitionType
	{
		PARTITION_WATERSHED,
		PARTITION_MONOTONE,
		PARTITION_LAYERS,
	};

	struct Builder
	{
		struct Edge
		{
			Edge() = default;
			Edge(uint32_t s, uint32_t e) : start(s), end(e) {};

			uint32_t start = 0;
			uint32_t end = 0;

			std::optional<uint32_t> portalToTriIndex;
		};

		struct Triangle
		{
			Triangle() = default;
			Triangle(const Triangle& rhs)
			{ 
				edges[0] = rhs.edges[0];
				edges[1] = rhs.edges[1];
				edges[2] = rhs.edges[2];
				index = rhs.index;
			}
			
			std::vector<uint32_t> GetVerts() const
			{
				std::vector<uint32_t> result;

				for (const auto& e : edges)
				{
					result.emplace_back(e.start);
				}
				return result;
			};

			void SetPortals(Triangle& other)
			{
				for (auto& mE : edges)
				{
					for (auto& oE : other.edges)
					{
						if (mE.start == oE.end && mE.end == oE.start && 
							!mE.portalToTriIndex.has_value() && !oE.portalToTriIndex.has_value())
						{
							mE.portalToTriIndex = other.index;
							oE.portalToTriIndex = index;
						}
					}
				}
			}

			std::array<Edge, 3> edges;
			uint32_t index = 0;
		};
	};

	class NavMeshBuilder
	{
	public:
		NavMeshBuilder();
		~NavMeshBuilder() = default;

		bool BuildNavMesh(float unitModifier = 10.f);
		bool BuildManualNavMesh(Ref<Mesh> geom);
		void ChangeMesh(InputGeom* geom);
		Ref<NavMesh> GetNavMesh() { return myNavMesh; }

		BuildContext* GetContext() { return &myCtx; }
		BuildSettings& GetBuildSettings() { return myBuildSettings; };

	private:
		void RemoveDublicates();
		void ConvertToVTNavMesh(float unitModifier);
		void CleanUp();

		Ref<NavMesh> myNavMesh;

		bool myKeepInterResults = false;
		float myTotalBuildTimeMs = 0.f;

		unsigned char* myTriareas;
		rcHeightfield* mySolid;
		rcCompactHeightfield* myChf;
		rcContourSet* myCset;
		rcPolyMesh* myPmesh;
		rcConfig myCfg;
		rcPolyMeshDetail* myDmesh;

		InputGeom* myGeom;
		BuildContext myCtx;
		
		BuildSettings myBuildSettings;

		bool myFilterLowHangingObstacles = true;
		bool myFilterLedgeSpans = true;
		bool myFilterWalkableLowHeightSpans = true;
	};
}