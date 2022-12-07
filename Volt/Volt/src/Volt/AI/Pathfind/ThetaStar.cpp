#include "vtpch.h"
#include "ThetaStar.h"

#include "Volt/Components/NavigationComponents.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	bool IsBridge(uint32_t a)
	{
		const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();

		return a >= verts.size();
	}

	bool IsInSet(const uint32_t index, const std::multiset<::ThetaStar::Node>& set)
	{
		for (const auto& node : set)
		{
			if (node.index == index) { return true; }
		}
		return false;
	}

	void RemoveFromSet(const uint32_t index, std::multiset<::ThetaStar::Node>& set)
	{
		for (auto i = set.begin(); i != set.end(); ++i)
		{
			if (i->index == index)
			{
				set.erase(i);
				break;
			}
		}
	}

	void RemoveFromStack(uint32_t index, std::stack<gem::vec3>& outPositions)
	{
		auto copy = outPositions;
		std::stack<gem::vec3> result;

		uint32_t i = (uint32_t)outPositions.size() - 1;
		while (!copy.empty())
		{
			if (index != i)
			{
				result.push(copy.top());
			}
			copy.pop();
			i--;
		}

		copy = result;
		result = std::stack<gem::vec3>();

		while (!copy.empty())
		{
			result.push(copy.top());
			copy.pop();
		}

		outPositions = result;
	}

	void InsertToStack(uint32_t index, gem::vec3 value, std::stack<gem::vec3>& outPositions)
	{
		auto copy = outPositions;
		std::stack<gem::vec3> result;

		uint32_t i = (uint32_t)outPositions.size() - 1;
		while (!copy.empty())
		{
			if (index == i)
			{
				result.push(value);
			}
			result.push(copy.top());
			copy.pop();
			i--;
		}

		copy = result;
		result = std::stack<gem::vec3>();

		while (!copy.empty())
		{
			result.push(copy.top());
			copy.pop();
		}

		outPositions = result;
	}

	gem::vec3 GetPositionFromStack(uint32_t index, const std::stack<gem::vec3>& aStack)
	{
		auto copy = aStack;

		uint32_t count = aStack.size() - 1;
		while (!copy.empty())
		{
			if (count == index)
			{
				return copy.top();
			}
			count--;
			copy.pop();
		}
	}

	NavMeshPath ThetaStar::FindPath(gem::vec3 current, gem::vec3 target)
	{
		/*
		// This main loop is the same as A*
		gScore(start) := 0
		parent(start) := start
		// Initializing open and closed sets. The open set is initialized
		// with the start node and an initial cost
		open := {}
		open.insert(start, gScore(start) + heuristic(start))
		// gScore(node) is the current shortest distance from the start node to node
		// heuristic(node) is the estimated distance of node from the goal node
		// there are many options for the heuristic such as Euclidean or Manhattan
		closed := {}
		while open is not empty
			s := open.pop()
			if s = goal
				return reconstruct_path(s)
			closed.push(s)
			for each neighbor of s
			// Loop through each immediate neighbor of s
				if neighbor not in closed
					if neighbor not in open
						// Initialize values for neighbor if it is
						// not already in the open list
						gScore(neighbor) := infinity
						parent(neighbor) := Null
					update_vertex(s, neighbor)
		return Null
		*/

		myScoreMap.clear();
		myParentMap.clear();
		myHeuristicMap.clear();
		myBridgeMap.clear();

		myOpen.clear();
		myClosed.clear();

		auto& navmesh = NavigationsSystem::Get().GetNavMesh();
		NavMeshPath finalPath;
		if (!navmesh) { return finalPath; }

		const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();
		const auto& indices = NavigationsSystem::Get().GetNavMesh()->GetIndices();

		myPolygon = CreatePolygon({ verts, indices });

		uint32_t start = GetClosestFromPosition(current);
		uint32_t end = GetClosestFromPosition(target);

		myScoreMap[start] = 0;
		myHeuristicMap[start] = Cost(start, end);
		myParentMap[start] = start;
		CreateBridgeMap();

		myOpen.insert({ start, myScoreMap.at(start) + myHeuristicMap.at(start) });

		while (!myOpen.empty())
		{
			auto s = (*myOpen.begin());
			RemoveFromSet(s.index, myOpen);
			if (s.index == end) 
			{
				finalPath.PathPositions.push(target);
				ReconstructPath(s.index, finalPath);
				finalPath.PathPositions.push(current);
				FixStartAndEnd(finalPath);
				break;
			}
			myClosed.insert(s);
			for (auto neighbor : GetNeighbors(s.index))
			{
				if (!IsInSet(neighbor, myClosed))
				{
					if (!IsInSet(neighbor, myOpen))
					{
						myScoreMap[neighbor] = (float)INT_MAX;
						myHeuristicMap[neighbor] = Cost(neighbor, end);
					}
					UpdateVertex(s.index, neighbor);
				}
			}
		}

		auto clonePath = finalPath;

		while (clonePath.PathPositions.size() > 1)
		{
			auto a = clonePath.PathPositions.top();
			clonePath.PathPositions.pop();
			auto b = clonePath.PathPositions.top();

			a.y = 100.f;
			b.y = 100.f;

			Renderer::SubmitLine(a, b);
		}

		//VT_TRACE(std::string("Start: ") + std::to_string(start));
		//VT_TRACE(std::string("End: ") + std::to_string(end));
		for (auto x : myParentMap)
		{
			//VT_TRACE(std::to_string(x.first) + std::string(" -> ") + std::to_string(x.second.value()));
		}

		return finalPath;
	}

	uint32_t ThetaStar::GetClosestFromPosition(gem::vec3 position)
	{
		// #SAMUEL_TODO: This function is problematic when spamming pathfinder.

		uint32_t index = 0;
		auto distance = (float)INT_MAX;

		auto sepPolys = myPolygon.SplitToSeperate();

		for (const auto& pol : sepPolys)
		{
			if (pol.IsInsideAny(gem::vec2(position.x, position.z)))
			{
				for (const auto& p : pol.points)
				{
					if (auto dist = gem::distance(position, p->position); dist < distance && InLineOfSight(position, p->position))
					{
						index = p->index;
						distance = dist;
					}
				}
			}
		}
		return index;
	}

	float ThetaStar::Cost(uint32_t a, uint32_t b)
	{
		const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();
		return gem::distance(
			(IsBridge(a)) ? myBridgeMap.find(a)->second.position : verts[a].position,
			(IsBridge(b)) ? myBridgeMap.find(b)->second.position : verts[b].position
		);
	}

	std::vector<uint32_t> ThetaStar::GetNeighbors(uint32_t a)
	{
		std::vector<uint32_t> neighbors;

		for (const auto& p : myPolygon.points)
		{
			if (p->index == a)
			{
				neighbors.push_back(p->next->index);
				neighbors.push_back(p->previous->index);
				break;
			}
		}

		for (const auto& bridge : myBridgeMap)
		{
			if (bridge.second.index == a)
			{
				neighbors.push_back(bridge.second.next);
				neighbors.push_back(bridge.second.previous);
				break;
			}
			else if (bridge.second.next == a || bridge.second.previous == a)
			{
				neighbors.push_back(bridge.second.index);
			}
		}

		return neighbors;
	}

	bool ThetaStar::InLineOfSight(uint32_t a, uint32_t b)
	{
		gem::vec4 redC(1.f, 0.f, 0.f, 1.f); // Delete later
		gem::vec4 greenC(0.f, 1.f, 0.f, 1.f); // Delete later

		const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();

		auto result = myPolygon.IsInLineOfSight(
			(IsBridge(a)) ? myBridgeMap.find(a)->second.position : verts[a].position,
			(IsBridge(b)) ? myBridgeMap.find(b)->second.position : verts[b].position
		);

		auto pos1 = (IsBridge(a)) ? myBridgeMap.find(a)->second.position : verts[a].position;
		auto pos2 = (IsBridge(b)) ? myBridgeMap.find(b)->second.position : verts[b].position;
		pos1.y = 100.f;
		pos2.y = 100.f;
		if (result)
		{
			Renderer::SubmitLine(pos1, pos2, greenC); // Delete later
		}
		else
		{
			Renderer::SubmitLine(pos1, pos2, redC); // Delete later
		}

		return result;
	}

	bool ThetaStar::InLineOfSight(gem::vec3 a, gem::vec3 b)
	{
		return myPolygon.IsInLineOfSight(a, b);
	}

	void ThetaStar::UpdateVertex(uint32_t s, uint32_t neighbor)
	{
		/*
		// This part of the algorithm is the main difference between A* and Theta*
		if line_of_sight(parent(s), neighbor)
			// If there is line-of-sight between parent(s) and neighbor
			// then ignore s and use the path from parent(s) to neighbor
			if gScore(parent(s)) + c(parent(s), neighbor) < gScore(neighbor)
				// c(s, neighbor) is the Euclidean distance from s to neighbor
				gScore(neighbor) := gScore(parent(s)) + c(parent(s), neighbor)
				parent(neighbor) := parent(s)
				if neighbor in open
					open.remove(neighbor)
				open.insert(neighbor, gScore(neighbor) + heuristic(neighbor))
		else
			// If the length of the path from start to s and from s to
			// neighbor is shorter than the shortest currently known distance
			// from start to neighbor, then update node with the new distance
			if gScore(s) + c(s, neighbor) < gScore(neighbor)
				gScore(neighbor) := gScore(s) + c(s, neighbor)
				parent(neighbor) := s
				if neighbor in open
					open.remove(neighbor)
				open.insert(neighbor, gScore(neighbor) + heuristic(neighbor))
		*/

		auto parent = myParentMap.at(s);
		if (InLineOfSight(parent.value(), neighbor))
		{
			if (auto newCost = myScoreMap.at(parent.value()) + Cost(parent.value(), neighbor); newCost < myScoreMap.at(neighbor))
			{
				myScoreMap[neighbor] = newCost;
				myParentMap[neighbor] = parent;
				if (IsInSet(neighbor, myOpen))
				{
					RemoveFromSet(neighbor, myOpen);
				}
				myOpen.insert({ neighbor, myScoreMap[neighbor] + myHeuristicMap[neighbor] });
			}
		}
		else
		{
			if (auto newCost = myScoreMap.at(s) + Cost(s, neighbor); newCost < myScoreMap.at(neighbor))
			{
				myScoreMap[neighbor] = newCost;
				myParentMap[neighbor] = s;
				if (IsInSet(neighbor, myOpen))
				{
					RemoveFromSet(neighbor, myOpen);
				}
				myOpen.insert({ neighbor, myScoreMap[neighbor] + myHeuristicMap[neighbor] });
			}
		}
	}

	void ThetaStar::ReconstructPath(uint32_t s, NavMeshPath& outPath)
	{
		/*
		total_path = {s}
		// This will recursively reconstruct the path from the goal node
		// until the start node is reached
		if parent(s) != s
			total_path.push(reconstruct_path(parent(s)))
		else
			return total_path
		*/

		const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();

		outPath.PathIds.push(s);
		outPath.PathPositions.push((IsBridge(s)) ? myBridgeMap.find(s)->second.position : verts[s].position);

		auto parent = myParentMap.at(s);
		if (parent.has_value() && parent.value() != s)
		{
			ReconstructPath(parent.value(), outPath);
		}
	}

	void ThetaStar::FixStartAndEnd(NavMeshPath& outPath)
	{
		// #SAMUEL_TODO: This works most of the times but have some edge cases where it doesn't

		bool removeStart = false, removeEnd = false;

		// Start
		auto pos1 = GetPositionFromStack(outPath.PathPositions.size() - 1, outPath.PathPositions);
		auto pos2 = GetPositionFromStack(outPath.PathPositions.size() - 3, outPath.PathPositions);
		if (InLineOfSight(pos1, pos2))
		{
			removeStart = true;
		}

		// End
		pos1 = GetPositionFromStack(0, outPath.PathPositions);
		pos2 = GetPositionFromStack(2, outPath.PathPositions);
		if (InLineOfSight(pos1, pos2))
		{
			removeEnd = true;
		}

		if (removeStart)
		{
			RemoveFromStack(outPath.PathPositions.size() - 2, outPath.PathPositions);
		}
		if (removeEnd)
		{
			if (outPath.PathPositions.size() > 2)
			{
				RemoveFromStack(1, outPath.PathPositions);
			}
		}
	}

	void ThetaStar::CreateBridgeMap()
	{
		//const auto& bridgeEntities = NavigationsSystem::Get().GetBridges();
		//auto bridgeSize = (uint32_t)bridgeEntities.size();

		//const auto& verts = NavigationsSystem::Get().GetNavMesh()->GetVertices();
		//auto vertsSize = (uint32_t)verts.size();

		//for (uint32_t i = 0; i < bridgeEntities.size(); i++)
		//{
		//	auto position = bridgeEntities[i].GetWorldPosition();
		//	auto bridgeComp = bridgeEntities[i].GetComponent<NavMeshBlockComponent>();
		//	if (bridgeComp.active && 
		//		myPolygon.IsInsideAny({ position.x + bridgeComp.from.x, position.z + bridgeComp.from.z }) &&
		//		myPolygon.IsInsideAny({ position.x + bridgeComp.to.x, position.z + bridgeComp.to.z }))
		//	{
		//		myBridgeMap[vertsSize + i].index = vertsSize + i;
		//		myBridgeMap[vertsSize + i].position = position + bridgeComp.from;
		//		myBridgeMap[vertsSize + i].previous = GetClosestFromPosition(position + bridgeComp.from);
		//		myBridgeMap[vertsSize + i].next = vertsSize + i + bridgeSize;

		//		myBridgeMap[vertsSize + i + bridgeSize].index = vertsSize + i + bridgeSize;
		//		myBridgeMap[vertsSize + i + bridgeSize].position = position + bridgeComp.to;
		//		myBridgeMap[vertsSize + i + bridgeSize].previous = vertsSize + i;
		//		myBridgeMap[vertsSize + i + bridgeSize].next = GetClosestFromPosition(position + bridgeComp.to);
		//	}
		//}
	}
}
