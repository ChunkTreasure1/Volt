#include "vtpch.h"
#include "pfFinder.h"
#include "pfNavMesh.h"
#include <limits>

namespace Pathfinder
{
	pfFinder::pfFinder(const pfNavMesh* navmesh)
	{
		m_navmesh = navmesh;
	}

	void pfFinder::reconstruct_path(uint32_t current, std::vector<uint32_t>& path)
	{
		path.emplace_back(current);

		if (auto parent = m_parents.at(current); parent != current)
		{
			reconstruct_path(parent, path);
			return;
		}

		std::reverse(path.begin(), path.end());
	}

	float pfFinder::Cost(uint32_t from, uint32_t to)
	{
		return Pathfinder::distance(m_navmesh->getCenter(from), m_navmesh->getCenter(to));
	}

	std::vector<uint32_t> pfFinder::AStar(uint32_t start, uint32_t end)
	{
		std::vector<uint32_t> path;
		if (!m_navmesh || start > m_navmesh->getPolyCount() - 1 || end > m_navmesh->getPolyCount() - 1) return path;

		std::vector<uint32_t> openSet;

		m_parents[start] = start;
		m_gScore[start] = 0.f;
		m_fScore[start] = Cost(start, end);

		openSet.emplace_back(start);

		while (!openSet.empty())
		{
			auto current = openSet.front();

			if (current == end)
			{
				reconstruct_path(current, path);
				return path;
			}

			openSet.erase(openSet.begin());

			for (const auto& link : m_navmesh->getLinks(current))
			{
				if (m_gScore.find(link.polyId) == m_gScore.end()) { m_gScore[link.polyId] = std::numeric_limits<float>::max(); }

				auto tentative_gScore = m_gScore.at(current) + Cost(current, link.polyId);
				if (tentative_gScore < m_gScore.at(link.polyId))
				{
					m_parents[link.polyId] = current;
					m_gScore[link.polyId] = tentative_gScore;
					m_fScore[link.polyId] = tentative_gScore + Cost(link.polyId, end);
					if (std::find(openSet.begin(), openSet.end(), link.polyId) == openSet.end())
					{
						bool inserted = false;
						for (uint32_t i = 0; const auto & id : openSet)
						{
							if (m_fScore.at(link.polyId) < m_fScore.at(id))
							{
								openSet.insert(openSet.begin() + i, link.polyId);
								inserted = true;
								break;
							}
							i++;
						}

						if (!inserted)
						{
							openSet.emplace_back(link.polyId);
						}
					}
				}
			}
		}

		uint32_t closest = start;
		float lowestCost = std::numeric_limits<float>::max();
		for (const auto& x : m_parents)
		{
			if (auto c = Cost(x.first, end); c < lowestCost)
			{
				lowestCost = c;
				closest = x.first;
			}
		}

		reconstruct_path(closest, path);

		return path;
	}

	float triarea2(const vec3 a, const vec3 b, const vec3 c)
	{
		const float ax = b.x - a.x;
		const float az = b.z - a.z;
		const float bx = c.x - a.x;
		const float bz = c.z - a.z;
		return bx * az - ax * bz;
	}

	bool vequal(const vec3& a, const vec3& b)
	{
		static const float eq = 0.001f;
		return Pathfinder::distance(a, b) < eq;
	}

	std::vector<vec3> pfFinder::Funnel(const std::vector<vec3>& path, const std::vector<pfLink>& portals)
	{
		if (path.empty() || portals.empty()) return path;

		std::vector<vec3> final_path;

		// Find straight path.
		int npts = 0;
		// Init scan state
		vec3 portalApex, portalLeft, portalRight;
		int apexIndex = 0, leftIndex = 0, rightIndex = 0;
		portalApex = path.front();
		auto portal_edge = m_navmesh->getLinkEdge(portals[0]);
		portalLeft = portal_edge.start;
		portalRight = portal_edge.end;

		// Add start point.
		final_path.emplace_back(portalApex);
		npts++;

		for (int i = 1; i < portals.size() && npts < path.size(); ++i)
		{
			portal_edge = m_navmesh->getLinkEdge(portals[i]);
			const vec3 left = portal_edge.start;
			const vec3 right = portal_edge.end;

			// Update right vertex.
			if (triarea2(portalApex, portalRight, right) <= 0.0f)
			{
				if (vequal(portalApex, portalRight) || triarea2(portalApex, portalLeft, right) > 0.0f)
				{
					// Tighten the funnel.
					portalRight = right;
					rightIndex = i;
				}
				else
				{
					// Right over left, insert left to path and restart scan from portal left point.
					final_path.emplace_back(portalLeft);
					npts++;
					// Make current left the new apex.
					portalApex = portalLeft;
					apexIndex = leftIndex;
					// Reset portal
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					// Restart scan
					i = apexIndex;
					continue;
				}
			}

			// Update left vertex.
			if (triarea2(portalApex, portalLeft, left) >= 0.0f)
			{
				if (vequal(portalApex, portalLeft) || triarea2(portalApex, portalRight, left) < 0.0f)
				{
					// Tighten the funnel.
					portalLeft = left;
					leftIndex = i;
				}
				else
				{
					// Left over right, insert right to path and restart scan from portal right point.
					final_path.emplace_back(portalRight);
					npts++;
					// Make current right the new apex.
					portalApex = portalRight;
					apexIndex = rightIndex;
					// Reset portal
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					// Restart scan
					i = apexIndex;
					continue;
				}
			}
		}

		// Append last point to path.
		if (npts < path.size())
		{
			final_path.emplace_back(path.back());
			npts++;
		}

		return final_path;
	}
}
