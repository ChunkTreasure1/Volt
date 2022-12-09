#include "vtpch.h"
#include "pfNavMesh.h"
#include "pfFinder.h"

#include <cassert>
#include <algorithm>

namespace Pathfinder
{

	uint32_t pfNavMesh::addVert(const vec3& vert)
	{
		m_verts.emplace_back(vert);
		return m_verts.size();
	}

	bool pfNavMesh::removeVert(const uint32_t& vertIndex)
	{
		if (vertIndex > m_verts.size() - 1 || vertIndex < 0) { return false; }
		m_verts.erase(m_verts.begin() + vertIndex);
		return true;
	}

	bool pfNavMesh::setVertices(const std::vector<vec3>& vertices)
	{
		m_verts.resize(vertices.size());
		m_verts = vertices;
		return true;
	}

	Pathfinder::pfPoly pfNavMesh::addPoly(const pfPoly& poly)
	{
		m_polygons.emplace_back(poly);
		return m_polygons.back();
	}

	bool pfNavMesh::removePoly(const uint32_t& polyIndex)
	{
		if (polyIndex > m_polygons.size() - 1 || polyIndex < 0) { return false; }
		m_polygons.erase(m_polygons.begin() + polyIndex);
		return true;
	}

	bool pfNavMesh::setPolygons(const std::vector<pfPoly>& polygons)
	{
		m_polygons.resize(polygons.size());
		m_polygons = polygons;
		return true;
	}

	std::vector<Pathfinder::vec3> pfNavMesh::findPath(const vec3& start, const vec3& end, std::vector<pfLink>* outPortals) const
	{
		pfFinder finder(this);

		auto startId = calcPolyLoc(start), endId = calcPolyLoc(end);
		auto path = finder.AStar(startId, endId);

		std::vector<vec3> pathPositions;
		std::vector<pfLink> portals;

		for (uint32_t i = 0; !path.empty() && i < path.size() - 1; i++)
		{
			pfLink portal;
			if (getLink(path[i], path[i + 1], portal))
			{
				portals.emplace_back(portal);
			}

			if (i % 2 == 0 || i < path.size() - 1)
			{
				pathPositions.emplace_back(getCenter(path[i]));
				pathPositions.emplace_back(getCenter(path[i + 1]));
			}
		}

		if (outPortals)
		{
			*outPortals = portals;
		}

		// #SAMUEL_TODO: Fix funnel to different heights
		pathPositions = finder.Funnel(pathPositions, portals);
		std::reverse(pathPositions.begin(), pathPositions.end());
		return pathPositions;
	}

	uint32_t pfNavMesh::calcPolyLoc(const vec3& pos) const
	{ 
		// #SAMUEL_TODO: Change this to check if inside triangle

		uint32_t closest = 0;
		float lowestDistance = std::numeric_limits<float>::max();
		for (uint32_t i = 0; i < getPolyCount(); i++)
		{
			if (auto dist = Pathfinder::distance(getCenter(i), pos); dist < lowestDistance)
			{
				lowestDistance = dist;
				closest = i;
			}
		}
		return closest;
	}

	Pathfinder::vec3 pfNavMesh::getCenter(uint32_t polyId) const
	{
		vec3 sum;
		for (const auto& v : m_polygons[polyId].verts)
		{
			sum += m_verts[v];
		}
		return sum / m_polygons[polyId].verts.size();
	}

	std::vector<Pathfinder::Line3f> pfNavMesh::getEdges() const
	{
		std::vector<Pathfinder::Line3f> edges;
		for (const auto& poly : m_polygons)
		{
			for (uint32_t i = 0; i < poly.verts.size(); i++)
			{
				Line3f line({ m_verts[poly.verts[i]] }, { m_verts[(i + 1 == poly.verts.size()) ? poly.verts[0] : poly.verts[i + 1]] });
				edges.emplace_back(line);
			}
		}
		return edges;
	}

	std::vector<Pathfinder::Line3f> pfNavMesh::getEdges(uint32_t polyId) const
	{
		std::vector<Pathfinder::Line3f> edges;
		for (uint32_t i = 0; i < m_polygons[polyId].verts.size(); i++)
		{
			Line3f line({ m_verts[m_polygons[polyId].verts[i]] }, { m_verts[(i + 1 == m_polygons[polyId].verts.size()) ? m_polygons[polyId].verts[0] : m_polygons[polyId].verts[i + 1]] });
			edges.emplace_back(line);
		}

		return edges;
	}

	Pathfinder::Line3f pfNavMesh::getLinkEdge(const pfLink& link) const
	{
		assert(link.edge[0] < getVertCount() && link.edge[1] < getVertCount(), "Indices exceeds vertex count");
		Line3f edge;

		edge.start = m_verts[link.edge[0]];
		edge.end = m_verts[link.edge[1]];

		return edge;
	}

	std::vector<Pathfinder::pfLink> pfNavMesh::getLinks() const
	{
		std::vector<Pathfinder::pfLink> links;
		for (const auto& poly : m_polygons)
		{
			links.insert(links.end(), poly.links.begin(), poly.links.end());
		}
		return links;
	}

	std::vector<Pathfinder::pfLink> pfNavMesh::getLinks(uint32_t polyId) const
	{
		return m_polygons[polyId].links;
	}

	bool pfNavMesh::getLink(uint32_t startPolyId, uint32_t endPolyId, pfLink& result) const
	{
		auto links = getLinks(startPolyId);
		for (const auto& link : links)
		{
			if (link.polyId == endPolyId)
			{
				result = link;
				return true;
			}
		}
		return false;
	}

	vec3 pfNavMesh::getVertex(uint32_t index) const
	{
		assert(index < m_verts.size());
		return m_verts[index];
	}

	const Pathfinder::pfPoly pfNavMesh::getPoly(uint32_t index) const
	{
		assert(index < m_polygons.size());
		return m_polygons[index];
	}

	std::vector<Pathfinder::vec3> pfNavMesh::getVertices() const
	{
		return m_verts;
	}

	std::vector<Pathfinder::pfPoly> pfNavMesh::getPolygons() const
	{
		return m_polygons;
	}
}
