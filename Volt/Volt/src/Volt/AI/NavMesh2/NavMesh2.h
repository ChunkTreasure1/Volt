#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/Vertex.h"

#include <GEM/gem.h>
#include <vector>

namespace Volt
{
	struct ConnectingSide
	{
		gem::vec3 to;
		gem::vec3 from;

		uint32_t fromIndex;
		uint32_t toIndex;
	};

	struct Line2
	{
		Line2() {};
		Line2(gem::vec3 s, gem::vec3 e)
		{
			start = s;
			end = e;
		}

		gem::vec3 start;
		gem::vec3 end;
	};

	struct Line2d
	{
		gem::vec2 start;
		gem::vec2 end;

		const gem::vec2 GetNormal() const
		{
			auto dir = gem::vec2(end.x - start.x, end.y - start.y);
			gem::vec2 tempVect = { -dir.y, dir.x };
			return gem::normalize(tempVect);
		}

		bool Inside(const gem::vec2& aPosition) const
		{
			float temp = gem::dot(aPosition, GetNormal()) - gem::dot(start, GetNormal());
			if (temp - std::numeric_limits<float>::epsilon() * 2 < 0)
			{
				return true;
			}
			return false;
		}
	};

	struct Triangle
	{
		gem::vec3 center;
		std::vector<Ref<Triangle>> adjecents;

		uint32_t indices[3];
		uint32_t index;
	};

	struct Node
	{
		gem::vec3 position;
		std::vector<uint32_t> adjacent;
		bool impassable = false;
		uint32_t index;
	};

	class Mesh;
	class NavMesh2
	{
	public:
		NavMesh2(Ref<Mesh> navMesh);

		std::vector<gem::vec3> FindPath(const gem::vec3& startPoint, const gem::vec3& endPoint);
		inline const std::vector<Ref<Triangle>>& GetTriangles() const { return myTriangles; }

		void Draw();

	private:
		void CalculateTriangles();
		bool LineIntersection(const Line2& lineA, const Line2& lineB, gem::vec3& intersectionPoint);

		int Heuristic(gem::vec3 aPoint, gem::vec3 aEndPoint);
		std::vector<uint32_t> PrintSolution(std::vector<int> dist, int index, std::vector<int> parents, int source);
		int FindMinimumDist(std::vector<int> someDistances, std::vector<bool> someValuesChecked, std::vector<Node> aMap);
		std::vector<uint32_t> AStar(const std::vector<Node>& aMap, int aStartIndex, int anEndIndex);

		Ref<Triangle> GetTriangleFromPoint(const gem::vec2& point);

		std::vector<ConnectingSide> myConnectingSides;
		std::vector<Ref<Triangle>> myTriangles;

		std::vector<Node> myMap;
		Ref<Mesh> myNavMesh;

		std::vector<Line2> myIntersectingLines;
		std::vector<Line2> myHeightDifferencePortals;
		std::vector<gem::vec3> myFunnelingResult;
		std::vector<gem::vec3> myLeftPoints;
		std::vector<gem::vec3> myRightPoints;

		std::vector<Line2> myLefts;
		std::vector<Line2> myRights;
		std::vector<Line2> myPortals;
		std::vector<Line2> myPath;

		uint32_t myPortalsDrawIndex = 0;
		std::vector<gem::vec3> draws;

		//std::vector<Vertex> myVertices;
		//std::vector<uint32_t> myIndices;
	};
}