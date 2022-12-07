#pragma once
#include "Volt/Core/Base.h"
#include "Volt/AI/NavMesh/NavigationsSystem.h"
#include "Volt/AI/NavMesh/Triangulation/Triangulation.h"

#include <optional>
#include <unordered_map>
#include <set>

namespace ThetaStar
{
	struct Bridge
	{
		uint32_t index = 0;
		uint32_t previous = 0;
		uint32_t next = 0;
		gem::vec3 position = 0.f;
	};

	struct Node
	{
		uint32_t index;
		float cost;

		bool operator<(const Node& rhs) const { return cost < rhs.cost; };
	};
}

namespace Volt
{
	class ThetaStar
	{
	public:
		ThetaStar() = delete;
		~ThetaStar() = delete;

		static NavMeshPath FindPath(gem::vec3 current, gem::vec3 target);

	private:
		static uint32_t GetClosestFromPosition(gem::vec3 position);
		static float Cost(uint32_t a, uint32_t b);
		static std::vector<uint32_t> GetNeighbors(uint32_t);

		static bool InLineOfSight(uint32_t a, uint32_t b);
		static bool InLineOfSight(gem::vec3 a, gem::vec3 b);
		static void UpdateVertex(uint32_t s, uint32_t neighbor);
		static void ReconstructPath(uint32_t s, NavMeshPath& outPath);
		static void FixStartAndEnd(NavMeshPath& outPath);
		static void CreateBridgeMap();

		inline static std::unordered_map<uint32_t, float> myScoreMap;
		inline static std::unordered_map<uint32_t, float> myHeuristicMap;
		inline static std::unordered_map<uint32_t, std::optional<uint32_t>> myParentMap;
		inline static std::unordered_map<uint32_t, ::ThetaStar::Bridge> myBridgeMap;

		inline static std::multiset<::ThetaStar::Node> myOpen;
		inline static std::multiset<::ThetaStar::Node> myClosed;

		inline static Polygon myPolygon;
	};
}