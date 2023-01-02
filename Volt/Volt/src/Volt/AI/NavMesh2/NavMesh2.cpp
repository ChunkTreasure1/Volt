#include "vtpch.h"
#include "NavMesh2.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/NavigationComponents.h"
#include "Volt/AI/NavMesh/NavigationsSystem.h"

namespace Volt
{
	NavMesh2::NavMesh2(Ref<Mesh> navMesh)
		: myNavMesh(navMesh)
	{
		CalculateTriangles();

		for (const auto& tri : myTriangles)
		{
			Node node;
			for (size_t i = 0; i < tri->adjecents.size(); i++)
			{
				node.adjacent.emplace_back(tri->adjecents[i]->index);
			}

			node.impassable = false;
			node.position = tri->center;
			node.index = tri->index;

			myMap.emplace_back(node);
		}
	}

	inline float triarea2(const gem::vec3& a, const gem::vec3& b, const gem::vec3& c)
	{
		float ax = b.x - a.x;
		float ay = b.z - a.z;
		float bx = c.x - a.x;
		float by = c.z - a.z;

		return bx * ay - ax * by;
	}

	inline float vdistsqr(const gem::vec3& a, const gem::vec3& b)
	{
		float x = b.x - a.x, y = b.z - a.z;
		return gem::sqrt(x * x + y * y);
	}

	inline bool vequal(const gem::vec3& a, const gem::vec3& b)
	{
		return vdistsqr(a, b) < (0.001f * 0.001f);
	}

	inline bool IsLeftOfLine(const gem::vec2& a, const gem::vec2& b, const gem::vec2& pos)
	{
		auto res = (pos.x - a.x) * (b.y - a.y) - (pos.y - b.y) * (b.x - a.x);
		return res;
	}

	inline bool PointInTriangle(const gem::vec2& p, const gem::vec2& tP0, const gem::vec2& tP1, const gem::vec2& tP2)
	{
		const float s = (tP0.x - tP2.x) * (p.y - tP2.y) - (tP0.y - tP2.y) * (p.x - tP2.x);
		const float t = (tP1.x - tP0.x) * (p.y - tP0.y) - (tP1.y - tP0.y) * (p.x - tP0.x);

		if ((s < 0.f) != (t < 0.f) && s != 0.f && t != 0.f)
		{
			return false;
		}

		const float d = (tP2.x - tP1.x) * (p.y - tP1.y) - (tP2.y - tP1.y) * (p.x - tP1.x);
		return d == 0.f || (d < 0.f) == (s + t <= 0.f);
	}

	std::vector<gem::vec3> NavMesh2::FindPath(const gem::vec3& startPoint, const gem::vec3& endPoint)
	{
		Line2d test;

		test.start = gem::vec2(1.f, 0.f);
		test.end = gem::vec2(2.f, 0.f);

		auto res = test.Inside(gem::vec2(1.5f, 1.f));
		res = test.Inside(gem::vec2(1.5f, -1.f));
		res = test.Inside(gem::vec2(1.5f, 0.f));
		res = test.Inside(gem::vec2(1.0f, 0.f));

		if (!myNavMesh) return {};

		uint32_t nearestStart = UINT32_MAX;
		float nearestStartDist = FLT_MAX;

		uint32_t nearestEnd = UINT32_MAX;
		float nearestEndDist = FLT_MAX;

		const auto& verts = myNavMesh->GetVertices();


		uint32_t nearestStartBackup = 0, nearestEndBackup = 0; // #SAMUEL_TODO: Remove this and add on line counts as inside below so ai doesn't freeze if they are on a line.
		bool foundStart = false, foundEnd = false;

		for (const auto& tri : myTriangles)
		{
			Line2d triL1 = { {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z}, {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z} };
			Line2d triL2 = { {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z}, {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z} };
			Line2d triL3 = { {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z}, {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z} };

			const auto startPointV2 = gem::vec2(startPoint.x, startPoint.z);
			const float dist = gem::distance(tri->center, startPoint);
			if (dist < nearestStartDist && triL1.Inside(startPointV2) && triL2.Inside(startPointV2) && triL3.Inside(startPointV2))
			{
				nearestStart = tri->index;
				nearestStartDist = dist;
				foundStart = true;
			}
			else if (dist < nearestStartDist)
			{
				//nearestStartBackup = tri->index;
				//nearestStartDist = dist;
			}
		}

		for (const auto& tri : myTriangles)
		{
			Line2d triL1 = { {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z}, {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z} };
			Line2d triL2 = { {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z}, {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z} };
			Line2d triL3 = { {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z}, {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z} };

			const auto endPointV2 = gem::vec2(endPoint.x, endPoint.z);
			const float dist = gem::distance(tri->center, endPoint);
			if (dist < nearestEndDist && triL1.Inside(endPointV2) && triL2.Inside(endPointV2) && triL3.Inside(endPointV2))
			{
				nearestEnd = tri->index;
				nearestEndDist = dist;
				foundEnd = true;
			}
			else if (dist < nearestEndDist)
			{
				//nearestEndBackup = tri->index;
				//nearestEndDist = dist;
			}
		}

		// #SAMUEL_TODO: This gets very laggy if updated everyframe
		for (const auto& ent : NavigationsSystem::Get().GetBridges())
		{
			auto bridgeComp = ent.GetComponent<NavMeshBlockComponent>();

			for (const auto& tri : myTriangles)
			{
				Line2d triL1 = { {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z}, {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z} };
				Line2d triL2 = { {verts[tri->indices[1]].position.x, verts[tri->indices[1]].position.z}, {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z} };
				Line2d triL3 = { {verts[tri->indices[2]].position.x, verts[tri->indices[2]].position.z}, {verts[tri->indices[0]].position.x, verts[tri->indices[0]].position.z} };

				const auto pointV2 = gem::vec2((ent.GetPosition() + bridgeComp.area).x, (ent.GetPosition() + bridgeComp.area).z);
				if (triL1.Inside(pointV2) && triL2.Inside(pointV2) && triL3.Inside(pointV2))
				{
					for (auto& x : myMap)
					{
						if (x.index == tri->index)
						{
							x.impassable = bridgeComp.active;
						}
					}
					break;
				}
			}
		}

		//if (!foundStart) { nearestStart = nearestStartBackup; }
		//if (!foundEnd) { nearestEnd = nearestEndBackup; }
		if (!foundStart) return { };
		if (!foundEnd) return { };

		std::vector<uint32_t> path = AStar(myMap, nearestStart, nearestEnd);

		std::vector<gem::vec3> result;
		for (unsigned int i : path)
		{
			result.emplace_back(myTriangles[i]->center);
		}

		std::vector<Line2> triangleLines;
		for (const auto& tri : myTriangles)
		{
			for (size_t j = 0; j < 3; j++)
			{
				uint32_t first = (tri->indices[j]);
				uint32_t second = (tri->indices[j + 1]);
				if (j == 2)
				{
					second = tri->indices[0];
				}

				triangleLines.emplace_back(verts[first].position, verts[second].position);
			}
		}

		myPath.clear();
		std::vector<Line2> pathLines;
		for (size_t i = 0; i < result.size(); i++)
		{
			if (i < result.size() - 1)
			{
				pathLines.emplace_back(result[i], result[i + 1]);

				auto debugPath = pathLines.back();
				debugPath.start.y += 50.f;
				debugPath.end.y += 50.f;
				myPath.emplace_back(debugPath);
			}
		}

		myIntersectingLines.clear();
		myHeightDifferencePortals.clear();
		std::vector<size_t> portalIntersectIndices;

		myPortals.clear();
		for (size_t i = 0; const auto & pathLine : pathLines)
		{
			for (uint32_t j = 0; const auto & triLine : myConnectingSides)
			{
				gem::vec3 pos;
				if (LineIntersection(pathLine, { triLine.from, triLine.to }, pos))
				{
					if ((int)pathLine.start.y != (int)pathLine.end.y 
						&& (int)triLine.from.y == (int)triLine.to.y)
					{
						myHeightDifferencePortals.emplace_back(triLine.from, triLine.to);
					}

					myIntersectingLines.emplace_back(triLine.from, triLine.to);
					portalIntersectIndices.emplace_back(i);

					auto debugPortal = myIntersectingLines.back();
					debugPortal.start.y += 50.f;
					debugPortal.end.y += 50.f;
					myPortals.emplace_back(debugPortal);
				}
				j++;
			}

			i++;
		}

		myIntersectingLines.emplace_back(endPoint, endPoint);

		for (size_t i = 0; i < myIntersectingLines.size() - 1; i++)
		{
			auto& line = myIntersectingLines.at(i);

			const auto& pathLine = pathLines.at(portalIntersectIndices.at(i));
			const gem::vec2 pathEnd = { pathLine.end.x, pathLine.end.z };
			const gem::vec2 pathStart = { pathLine.start.x, pathLine.start.z };

			const gem::vec2 lineDir = gem::normalize(gem::vec2{ line.end.x, line.end.z } - gem::vec2{ line.start.x, line.start.z });
			const gem::vec2 pathDir = gem::normalize(pathEnd - pathStart);

			Line2d portalLine = { pathStart, pathEnd };

			//if (IsLeftOfLine(pathDir, lineDir, { line.start.x, line.start.z }))
			if (portalLine.Inside({ line.start.x, line.start.z }))
			{
				std::swap(line.start, line.end);
			}

			//if (line.start.z > line.end.z)
			//{
			//	std::swap(line.start, line.end);
			//}
		}

		myFunnelingResult.clear();
		myLeftPoints.clear();
		myRightPoints.clear();

		const auto& portals = myIntersectingLines;
		std::vector<gem::vec3> funnelingResult;

		myLefts.clear();
		myRights.clear();

		for (const auto& port : portals)
		{
			myLeftPoints.emplace_back(port.start);
			myRightPoints.emplace_back(port.end);
		}

		// #SAMUEL_TODO: Fix so it doesn't fly up before reaching stairs
		// Funneling
		{
			gem::vec3 portalApex, portalLeft, portalRight;
			int32_t apexIndex = 0, leftIndex = 0, rightIndex = 0;

			portalApex = startPoint;
			if (portals.size() > 1)
			{
				portalLeft = portals[1].start;
				portalRight = portals[1].end;
			}
			else if (!path.empty())
			{
				funnelingResult.emplace_back(portals.front().start);
			}

			for (size_t i = 1; i < portals.size(); i++)
			{
				gem::vec3 left = portals[i].start;
				gem::vec3 right = portals[i].end;

				Line2 leftLine;
				Line2 rightLine;

				leftLine.start = portalApex;
				leftLine.end = left;

				rightLine.start = portalApex;
				rightLine.end = right;

				leftLine.start.y += 50.f;
				leftLine.end.y += 50.f;

				rightLine.start.y += 50.f;
				rightLine.end.y += 50.f;

				myLefts.emplace_back(leftLine);
				myRights.emplace_back(rightLine);

				// Update right vertex
				if (triarea2(portalApex, portalRight, right) <= 0.f)
				{
					if (vequal(portalApex, portalRight) || triarea2(portalApex, portalLeft, right) > 0.f)
					{
						//if ((int)portalApex.y != (int)right.y)
						//{
						//	Line2 funnel(portalApex, endPoint); // Endpoint won't work here
						//	Line2 portal(portals[i - 1].start, portals[i - 1].end);

						//	gem::vec3 intersectionPoint;
						//	if (LineIntersection(funnel, portal, intersectionPoint))
						//	{
						//		funnelingResult.emplace_back(intersectionPoint);
						//	}
						//}

						portalRight = right;
						rightIndex = i;
					}
					else
					{
						// Right over left, insert left and restart from left point
						funnelingResult.emplace_back(portalLeft);
						portalApex = portalLeft;
						apexIndex = leftIndex;

						// Reset
						portalLeft = portalApex;
						portalRight = portalApex;
						leftIndex = apexIndex;
						rightIndex = apexIndex;

						i = apexIndex;
						continue;
					}
				}

				// Update left vertex
				if (triarea2(portalApex, portalLeft, left) >= 0.f)
				{
					if (vequal(portalApex, portalLeft) || triarea2(portalApex, portalRight, left) < 0.f)
					{
						//if ((int)portalApex.y != (int)left.y)
						//{
						//	Line2 funnel(portalApex, endPoint); // Endpoint won't work here
						//	Line2 portal(portals[i - 1].start, portals[i - 1].end);

						//	gem::vec3 intersectionPoint;
						//	if (LineIntersection(funnel, portal, intersectionPoint))
						//	{
						//		funnelingResult.emplace_back(intersectionPoint);
						//	}
						//}

						portalLeft = left;
						leftIndex = i;
					}
					else
					{
						// Right over left, insert left and restart from left point
						funnelingResult.emplace_back(portalRight);
						portalApex = portalRight;
						apexIndex = rightIndex;

						// Reset
						portalLeft = portalApex;
						portalRight = portalApex;
						leftIndex = apexIndex;
						rightIndex = apexIndex;

						i = apexIndex;
						continue;
					}
				}
			}
		}

		if (!funnelingResult.empty() && (!vequal(funnelingResult.back(), portals.back().start)))
		{
			funnelingResult.emplace_back(portals.back().end);
		}

		funnelingResult.insert(funnelingResult.begin(), startPoint);
		auto newFunnel = funnelingResult;

		uint32_t insertOffset = 0;
		for (uint32_t i = 1; i < funnelingResult.size(); ++i)
		{
			if ((int)funnelingResult[i].y != (int)funnelingResult[i - 1].y)
			{
				Line2 funnel(funnelingResult[i - 1], funnelingResult[i]);

				for (const auto& portal : myHeightDifferencePortals)
				{
					gem::vec3 pos;

					if (LineIntersection(funnel, portal, pos))
					{
						newFunnel.insert(newFunnel.begin() + i + insertOffset, pos);
						insertOffset++;
					}
				}
			}
		}

		funnelingResult = newFunnel;

		float entirePathDist = 0;

		for (uint32_t i = 1; i < funnelingResult.size(); ++i)
		{
			if (gem::abs(funnelingResult[i - 1].y - funnelingResult[i].y) < 30.f)
			{
				funnelingResult[i].y = funnelingResult[i - 1].y;
			}
			entirePathDist += gem::distance(funnelingResult[i - 1], funnelingResult[i]);
		}

		VT_CORE_INFO(std::format("Path Distance: {0}units", entirePathDist));
		static float maxPathDist = 5000.f;
		if (entirePathDist > maxPathDist)
		{
			return std::vector<gem::vec3>();
		}

		std::reverse(funnelingResult.begin(), funnelingResult.end());
		funnelingResult.pop_back();

		// Prune
		//if (funnelingResult.size() > 2)
		//{
		//	std::vector<size_t> pointsToBeRemoved;

		//	for (size_t i = 0; i < funnelingResult.size() - 2; i++)
		//	{
		//		const gem::vec2 currPos = { funnelingResult[i].x, funnelingResult[i].z };
		//		const gem::vec2 nextPos = { funnelingResult[i + 2].x, funnelingResult[i + 2].z };

		//		const Ref<Triangle> currentTriangle = GetTriangleFromPoint(currPos);

		//		const gem::vec2 dirToNext = gem::normalize(nextPos - currPos);
		//		const float distanceToNext = gem::distance(currPos, nextPos);

		//		const float checkCount = 10.f;
		//		const float perCheckDist = distanceToNext / checkCount;

		//		// Ray marching
		//		const auto& vertices = myNavMesh->GetVertices();

		//		bool outside = false;
		//		for (float t = perCheckDist; t <= distanceToNext; t += perCheckDist)
		//		{
		//			const gem::vec2 currRayPos = currPos + dirToNext * t;
		//			

		//		}

		//		if (!outside)
		//		{
		//			pointsToBeRemoved.emplace_back(i + 1);
		//		}
		//	}

		//	for (const auto& index : pointsToBeRemoved)
		//	{
		//		funnelingResult.erase(funnelingResult.begin() + index);
		//	}
		//}

		myFunnelingResult = funnelingResult;
		return funnelingResult;
	}

	bool NavMesh2::LineIntersection(const Line2& lineA, const Line2& lineB, gem::vec3& intersectionPoint)
	{
		float s1_x, s1_y, s2_x, s2_y, s1_z;
		s1_x = lineA.end.x - lineA.start.x; s1_y = lineA.end.z - lineA.start.z;
		s2_x = lineB.end.x - lineB.start.x; s2_y = lineB.end.z - lineB.start.z;

		float s, t;
		s = (-s1_y * (lineA.start.x - lineB.start.x) + s1_x * (lineA.start.z - lineB.start.z)) / (-s2_x * s1_y + s1_x * s2_y);
		t = (s2_x * (lineA.start.z - lineB.start.z) - s2_y * (lineA.start.x - lineB.start.x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			// Collision detected
			intersectionPoint.x = lineA.start.x + (t * s1_x);
			intersectionPoint.y = lineB.start.y;
			intersectionPoint.z = lineA.start.z + (t * s1_y);
			return true;
		}

		return false; // No collision
	}

	//static const int MapWidth = 20;
//static const int MapHeight = 20;
//static const int TileCount = MapWidth * MapHeight;

	int NavMesh2::Heuristic(gem::vec3 aPoint, gem::vec3 aEndPoint)
	{
		//auto resolution = Tga2D::CEngine::GetInstance()->GetTargetSize();
		//aPoint = {aPoint.x* resolution.x,aPoint.y*resolution.y};
		//aEndPoint = { aEndPoint.x* resolution.x,aEndPoint.y*resolution.y};

		int heuristic = static_cast<int>(gem::distance(aPoint, aEndPoint));
		return heuristic;
	}

	void NavMesh2::Draw()
	{
		//for (const auto& tri : myTriangles)
		//{
		//	const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, tri->center) * gem::scale(gem::mat4{ 1.f }, gem::vec3{ 0.1f });
		//	Renderer::Submit(AssetManager::GetAsset<Mesh>("Assets/Meshes/Primitives/Cube.vtmesh"), transform);
		//}

		for (uint32_t i = 0; const auto & vert : myLeftPoints)
		{
			const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, { vert.x, 50.f, vert.z }) * gem::scale(gem::mat4{ 1.f }, gem::vec3{ 0.3f });
			Renderer::Submit(AssetManager::GetAsset<Mesh>("Assets/Meshes/Primitives/Cube.vtmesh"), AssetManager::GetAsset<Material>("Assets/Materials/M_BloomTest.vtmat"), transform);
		}

		for (const auto& vert : myRightPoints)
		{
			const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, { vert.x, 50.f, vert.z }) * gem::scale(gem::mat4{ 1.f }, gem::vec3{ 0.3f });
			Renderer::Submit(AssetManager::GetAsset<Mesh>("Assets/Meshes/Primitives/Cube.vtmesh"), transform);
		}

		//for (const auto& vert : myLeftPoints)
		//{
		//	const gem::mat4 transform = gem::translate(gem::mat4{ 1.f }, { vert.x, 50.f, vert.z }) * gem::scale(gem::mat4{ 1.f }, gem::vec3{ 0.3f });
		//	Renderer::Submit(AssetManager::GetAsset<Mesh>("Assets/Meshes/Primitives/Cube.vtmesh"), AssetManager::GetAsset<Material>("Assets/Materials/M_BloomTest.vtmat"), transform);
		//}

		//for (uint32_t i = 0; i < draws.size(); i++)
		//{
		//	if (i != draws.size() - 1)
		//	{
		//		Renderer::SubmitLine(draws[i], draws[i + 1]);
		//	}
		//}

		//for (size_t i = 0; i < indices.size(); i += 3)
		//{
		//	if (indices.size() > i + 3)
		//	{
		//		for (size_t j = 0; j < 3; j++)
		//		{
		//			uint32_t first = (indices[i + j]);
		//			uint32_t second = (indices[i + j + 1]);
		//			if (j == 2)
		//			{
		//				second = (indices[i]);
		//			}

		//			Renderer::SubmitLine(vertices[first].position, vertices[second].position);
		//		}
		//	}
		//}

		if (Volt::Input::IsKeyPressed(VT_KEY_8))
		{
			if (myPortalsDrawIndex != 0)
			{
				myPortalsDrawIndex--;
			}
		}
		if (Volt::Input::IsKeyPressed(VT_KEY_9))
		{
			if (myPortalsDrawIndex < myPortals.size())
			{
				myPortalsDrawIndex++;
			}
			else
			{
				myPortalsDrawIndex = 0;
			}
		}
		if (Volt::Input::IsKeyPressed(VT_KEY_0))
		{
			myPortalsDrawIndex = 0;
		}

		for (size_t i = 0; i < myPath.size(); i++)
		{
			Renderer::SubmitLine(myPath[i].start, myPath[i].end);
		}

		for (size_t i = 0; i < myLefts.size(); i++)
		{
			if (i != myPortalsDrawIndex) { continue; }
			Renderer::SubmitLine(myLefts[i].start, myLefts[i].end, { 1.f, 0.f, 0.f, 1.f });
		}

		for (size_t i = 0; i < myRights.size(); i++)
		{
			if (i != myPortalsDrawIndex) { continue; }
			Renderer::SubmitLine(myRights[i].start, myRights[i].end, { 0.f, 0.f, 1.f, 1.f });
		}

		for (size_t i = 0; i < myPortals.size(); i++)
		{
			//Renderer::SubmitLine(myPortals[i].start, myPortals[i].end, { 0.f, 1.f, 0.f, 1.f });
		}

		for (size_t i = 0; i < myHeightDifferencePortals.size(); i++)
		{
			Renderer::SubmitLine(myHeightDifferencePortals[i].start, myHeightDifferencePortals[i].end, { 0.f, 1.f, 0.f, 1.f });
		}

		for (size_t i = 0; i < myIntersectingLines.size(); i++)
		{
			//Renderer::SubmitLine(myIntersectingLines[i].start, myIntersectingLines[i].end, { 1.f, 0.f, 0.f, 1.f });
		}
	}

	inline bool printPath(std::vector<int> path, int j, int source, std::vector<uint32_t>& endPath)
	{
		if (path[j] == INT_MAX)
		{
			return false;
		}
		if (path[j] == source)
		{
			endPath.push_back(j);
			return true;
		}
		if (j == path[path[j]])
		{
			return false;
		}
		endPath.push_back(j);
		return printPath(path, path[j], source, endPath);
	}

	std::vector<uint32_t> NavMesh2::PrintSolution(std::vector<int> dist, int index, std::vector<int> parents, int source)
	{
		std::vector<uint32_t> endPath = {};
		while (true)
		{
			if (printPath(parents, index, source, endPath))
			{
				//std::cout << "Found path!" << std::endl;
				endPath.push_back(source);
				std::reverse(endPath.begin(), endPath.end());
			}
			else
			{
				//std::cout << "Found no path" << std::endl;
				endPath.clear();
			}
			break;
		}
		return endPath;
	}

	int NavMesh2::FindMinimumDist(std::vector<int> someDistances, std::vector<bool> someValuesChecked, std::vector<Node> aMap)
	{
		int min = INT_MAX;
		int min_index = 0;

		for (int i = 0; i < someValuesChecked.size(); i++)
		{
			if (someValuesChecked[i] == false && someDistances[i] <= min && !aMap[i].impassable)
			{
				min = someDistances[i];
				min_index = i;
			}
		}

		return min_index;
	}


	std::vector<uint32_t> NavMesh2::AStar(const std::vector<Node>& aMap, int aStartIndex, int anEndIndex)
	{
		std::vector<int> gCost;
		std::vector<int> fCost;
		std::vector<int> path;
		std::vector<int> parents;
		std::vector<bool> hasBeenChecked;
		std::vector<int> amountLeft;

		for (int i = 0; i < aMap.size(); i++)
		{
			gCost.push_back(INT_MAX);
			fCost.push_back(INT_MAX);
			parents.push_back(INT_MAX);
			hasBeenChecked.push_back(false);
			amountLeft.push_back(i);
		}

		gCost[aStartIndex] = 0;
		fCost[aStartIndex] = Heuristic(aMap[aStartIndex].position, aMap[anEndIndex].position); // change heuristic to be pos based

		int currentIndex = 0;
		while (amountLeft.size() > 0)
		{
			currentIndex = FindMinimumDist(fCost, hasBeenChecked, aMap);

			//std::vector<int> neighbours = UpdateAdjacentTiles(aMap, currentIndex);
			if (currentIndex == anEndIndex)
			{
				if (path.empty())
				{
					gCost[anEndIndex] = 0;
					fCost[anEndIndex] = 0;
					parents[anEndIndex] = anEndIndex;
					path.emplace_back(anEndIndex);
				}
				return PrintSolution(gCost, anEndIndex, parents, aStartIndex);
			}
			hasBeenChecked[currentIndex] = true;
			amountLeft.erase(amountLeft.begin());

			for (auto& var : aMap[currentIndex].adjacent)
			{
				int dist = gCost[currentIndex] + 1 /*aMap[var].weight*/;

				if (dist < gCost[var])
				{
					gCost[var] = dist;
					fCost[var] = dist + Heuristic(aMap[var].position, aMap[anEndIndex].position);
					path.push_back(var);
					parents[var] = currentIndex;
				}
			}
		}

		std::vector<uint32_t> noPathFound;
		return noPathFound;
	}

	Ref<Triangle> NavMesh2::GetTriangleFromPoint(const gem::vec2& point)
	{
		const auto& vertices = myNavMesh->GetVertices();

		for (const auto& tri : myTriangles)
		{
			const auto& vp0 = vertices[tri->indices[0]];
			const auto& vp1 = vertices[tri->indices[1]];
			const auto& vp2 = vertices[tri->indices[2]];

			const gem::vec2 p0 = { vp0.position.x, vp0.position.z };
			const gem::vec2 p1 = { vp1.position.x, vp1.position.z };
			const gem::vec2 p2 = { vp2.position.x, vp2.position.z };

			if (PointInTriangle(point, p0, p1, p2))
			{
				return tri;
			}
		}

		return nullptr;
	}

	void NavMesh2::CalculateTriangles()
	{
		const auto& indices = myNavMesh->GetIndices();
		const auto& vertices = myNavMesh->GetVertices();

		//auto x = CreatePolygon({vertices, indices});

		//for (const auto& p : x.points)
		//{
		//	draws.emplace_back(p->position);
		//}

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			Ref<Triangle> node = CreateRef<Triangle>();
			for (size_t j = 0; j < 3; j++)
			{
				uint32_t first = indices[i + j];
				node->indices[j] = first;
			}

			const float x = (vertices[node->indices[0]].position.x + vertices[node->indices[1]].position.x + vertices[node->indices[2]].position.x) / 3.f;
			const float y = (vertices[node->indices[0]].position.y + vertices[node->indices[1]].position.y + vertices[node->indices[2]].position.y) / 3.f;
			const float z = (vertices[node->indices[0]].position.z + vertices[node->indices[1]].position.z + vertices[node->indices[2]].position.z) / 3.f;

			node->center = { x, y, z };
			node->index = (uint32_t)myTriangles.size();
			myTriangles.emplace_back(node);
		}

		for (size_t i = 0; i < myTriangles.size(); i++)
		{
			for (size_t j = 0; j < myTriangles.size(); j++)
			{
				if (myTriangles[i] != myTriangles[j])
				{
					bool checked = false;
					for (size_t h = 0; h < myTriangles[j]->adjecents.size(); h++)
					{
						if (myTriangles[j]->adjecents[h] == myTriangles[i])
						{
							checked = true;
							break;
						}
					}
					if (!checked)
					{
						for (size_t h = 0; h < 3; h++)
						{
							size_t secondH = h + 1;
							size_t pastH = h - 1;
							if (h == 2)
							{
								secondH = 0;
							}
							if (h == 0)
							{
								pastH = 2;
							}
							for (size_t k = 0; k < 3; k++)
							{
								size_t secondK = k + 1;
								size_t pastK = k - 1;

								if (k == 2)
								{
									secondK = 0;
								}
								if (k == 0)
								{
									pastK = 2;
								}

								if (vertices[myTriangles[i]->indices[h]].ComparePosition(vertices[myTriangles[j]->indices[k]])) // cursed i hate it
								{
									bool hasPassed = false;
									uint32_t toIndex = 0;
									uint32_t fromIndex = myTriangles[i]->indices[h];

									if (vertices[myTriangles[i]->indices[secondH]].ComparePosition(vertices[myTriangles[j]->indices[secondK]]))
									{
										toIndex = myTriangles[i]->indices[secondH];
										hasPassed = true;
									}
									if (vertices[myTriangles[i]->indices[pastH]].ComparePosition(vertices[myTriangles[j]->indices[pastK]]))
									{
										toIndex = myTriangles[i]->indices[pastH];

										hasPassed = true;

									}
									if (vertices[myTriangles[i]->indices[secondH]].ComparePosition(vertices[myTriangles[j]->indices[pastK]]))
									{
										toIndex = myTriangles[i]->indices[secondH];

										hasPassed = true;

									}
									if (vertices[myTriangles[i]->indices[pastH]].ComparePosition(vertices[myTriangles[j]->indices[secondK]]))
									{
										toIndex = myTriangles[i]->indices[pastH];

										hasPassed = true;

									}

									if (std::find_if(myConnectingSides.begin(), myConnectingSides.end(), [&](const ConnectingSide& side)
										{
											return side.fromIndex == fromIndex && side.toIndex == toIndex || side.fromIndex == toIndex && side.toIndex == fromIndex;
										}) != myConnectingSides.end())
									{
										break;
									}

										if (hasPassed)
										{
											myTriangles[i]->adjecents.push_back(myTriangles[j]);
											myTriangles[j]->adjecents.push_back(myTriangles[i]);
											ConnectingSide side;
											side.from = vertices[fromIndex].position;
											side.to = vertices[toIndex].position;
											side.fromIndex = fromIndex;
											side.toIndex = toIndex;
											myConnectingSides.push_back(side);
											break;
										}
								}

							}
						}
					}
				}
			}
		}


	}
}