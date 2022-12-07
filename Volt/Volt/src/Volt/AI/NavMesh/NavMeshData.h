#pragma once
#include "Volt/Asset/Asset.h"
#include <Volt/Rendering/Vertex.h>

#include <gem/gem.h>
#include <vector>
#include <stack>

#define VertsPerPoly 3

namespace Volt
{
	typedef uint32_t CellID;
	constexpr CellID NullCell = 0;
	typedef uint32_t MeshIndex;
	typedef std::pair<std::vector<Vertex>, std::vector<MeshIndex>> MeshInfo;

	struct NavMeshVertex
	{
		NavMeshVertex() = default;
		NavMeshVertex(const Vertex& aVertex)
		{
			Position = aVertex.position;
		};
		gem::vec3 Position = { 0.f, 0.f, 0.f };
	};

	struct NavMeshCell
	{
		CellID Id = 0;
		uint32_t numNeighbours = 0;
		gem::vec3 Center = { 0.f, 0.f, 0.f };
		std::array<CellID, VertsPerPoly> NeighbourCells = { NullCell, NullCell, NullCell };
		std::array<gem::vec3, VertsPerPoly> NeighbourPortals = { gem::vec3(0.f), gem::vec3(0.f), gem::vec3(0.f) };
		std::array<MeshIndex, VertsPerPoly> Indices = { 0, 0, 0 };
	};

	struct NavMesh
	{
		NavMesh() = default;
		NavMesh(const std::vector<NavMeshVertex>& aVertices, const std::vector<MeshIndex>& aIndices)
		{
			Vertices = aVertices;
			Indices = aIndices;
		};
		NavMesh(const std::vector<NavMeshVertex>& aVertices, const std::vector<MeshIndex>& aIndices, const std::vector<NavMeshCell>& aCells)
		{
			Vertices = aVertices;
			Indices = aIndices;
			Cells = aCells;
		};

		std::vector<NavMeshVertex> Vertices;
		std::vector<MeshIndex> Indices;
		std::vector<NavMeshCell> Cells;
	};

	struct NavMeshPath
	{
		std::stack<CellID> PathIds;
		std::stack<gem::vec3> PathPositions;
	};
}