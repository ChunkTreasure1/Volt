#pragma once
#include "Math/pfMath.h"

#include <stdint.h>
#include <vector>
#include <memory>

namespace Pathfinder
{
	class pfLink
	{
	public:
		pfLink() = default;
		~pfLink() = default;

		uint32_t edge[2]; // Indices to m_verts in pfNavMesh
		uint32_t polyId = 0; // Index to m_polygons in pfNavMesh
	};

	class pfPoly
	{
	public:
		pfPoly() = default;
		~pfPoly() = default;

		std::vector<uint32_t> verts; // Indices to m_verts in pfNavMesh
		std::vector<pfLink> links; // Links to other pfPolys
	};

	class pfNavMesh
	{
	public:
		pfNavMesh() = default;
		~pfNavMesh() = default;

		uint32_t addVert(const vec3& vert);
		bool removeVert(const uint32_t& vertIndex);
		bool setVertices(const std::vector<vec3>& vertices);

		pfPoly addPoly(const pfPoly& poly); // Returns ref to poly if successful otherwise nullptr.
		bool removePoly(const uint32_t& polyIndex);
		bool setPolygons(const std::vector<pfPoly>& polygons);

		std::vector<vec3> findPath(const vec3& start, const vec3& end, std::vector<pfLink>* outPortals = nullptr) const;

		uint32_t calcPolyLoc(const vec3& pos) const; // Returns polyId from coords.

		vec3 getCenter(uint32_t polyId) const;

		std::vector <Line3f> getEdges() const; // Returns all edges
		std::vector<Line3f> getEdges(uint32_t polyId) const; // Returns specific edges from polygon

		Line3f getLinkEdge(const pfLink& link) const;
		std::vector<pfLink> getLinks() const; // Returns all links
		std::vector<pfLink> getLinks(uint32_t polyId) const; // Returns specific links from polygon
		bool getLink(uint32_t startPolyId, uint32_t endPolyId, pfLink& result) const; // Returns specific link from polygons

		vec3 getVertex(uint32_t index) const;
		const pfPoly getPoly(uint32_t index) const;

		std::vector<vec3> getVertices() const;
		std::vector<pfPoly> getPolygons() const;

		uint32_t getVertCount() const { return m_verts.size(); };
		uint32_t getPolyCount() const { return m_polygons.size(); };

	private:
		std::vector<vec3> m_verts;
		std::vector<pfPoly> m_polygons;
	};

}