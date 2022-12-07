#pragma once
#include "Volt/AI/NavMesh/NavMeshData.h"
#include "Volt/AI/NavMesh/Triangulation/TriangulationStructs.h"

#include <vector>

namespace TriangulationUtils
{
	bool contains(const std::vector<uint32_t>& vec, const uint32_t& elem);
}

//struct ConnectingSide
//{
//	gem::vec3 to;
//	gem::vec3 from;
//
//	uint32_t fromIndex;
//	uint32_t toIndex;
//};

//struct Triangle
//{
//	gem::vec3 center;
//	std::vector<Ref<Triangle>> adjecents;
//
//	uint32_t indices[3];
//	std::vector<ConnectingSide> connections;
//	uint32_t index;
//};

namespace Volt
{
	Polygon CreatePolygon(const MeshInfo& meshInfo);
	bool MergePolygons(const Polygon& a, const Polygon& b, Polygon& outPoly);
	Polygon MergePolygons(const std::vector<Polygon>& polygons);
	std::vector<uint32_t> TriangulatePolygon(const Polygon& polygon);
	bool ValidatePolygon(const Polygon& polygon);
	//std::vector<Ref<Triangle>> CalculateTriangles(const MeshInfo& meshInfo);
}