#include "vtpch.h"
#include "Triangulation.h"
#include "Volt/Log/Log.h"
#include "Volt/AI/NavMesh/Math/LineVolume.hpp"

namespace TriangulationUtils
{
	bool contains(const std::vector<uint32_t>& vec, const uint32_t& elem)
	{
		bool result = false;
		if (std::find(vec.begin(), vec.end(), elem) != vec.end())
		{
			result = true;
		}
		return result;
	}
}

namespace Volt
{
	bool IsInTriangle(Ref<Point> point, Ref<Point> v1, Ref<Point> v2, Ref<Point> v3)
	{
		gem::vec3 p = point->position;
		gem::vec3 a = v1->position;
		gem::vec3 b = v2->position;
		gem::vec3 c = v3->position;

		a -= p;
		b -= p;
		c -= p;

		p -= p;

		gem::vec3 u = gem::cross(b, c);
		gem::vec3 v = gem::cross(c, a);
		gem::vec3 w = gem::cross(a, b);

		if (gem::dot(u, v) < 0.f)
		{
			return false;
		}
		if (gem::dot(u, w) < 0.0f)
		{
			return false;
		}

		return true;
	}

	std::vector<Ref<Point>> GetPointsInTriangle(Polygon polygon, Ref<Point> v1, Ref<Point> v2, Ref<Point> v3)
	{
		std::vector<Ref<Point>> result;

		for (const auto& point : polygon.points)
		{
			if (point->index == v1->index || point->index == v2->index || point->index == v3->index) { continue; }
			if (IsInTriangle(point, v1, v2, v3))
			{
				result.emplace_back(point);
			}
		}

		return result;
	}

	bool IsConvex(Ref<Point> point)
	{
		Point a = *point->previous;
		Point b = *point;
		Point c = *point->next;

		auto res = ((a.position.x * (c.position.z - b.position.z)) + (b.position.x * (a.position.z - c.position.z)) + (c.position.x * (b.position.z - a.position.z)));
		return res > 0;
	}

	Ref<Point> GetFarthestPointFromLine(const std::vector<Ref<Point>>& pointsInTriangle, Ref<Point> startLine, Ref<Point> endLine)
	{
		Ref<Point> farthestPoint;
		float farthestDistance = 0.f;

		gem::vec3 line = endLine->position - startLine->position;

		for (const auto& point : pointsInTriangle)
		{
			if (point->index == startLine->index || point->index == endLine->index || point->index == startLine->previous->index) { continue; }

			auto startToPoint = point->position - startLine->position;
			auto pointToLine = startToPoint - line * gem::dot(startToPoint, line) / gem::dot(line, line);
			auto magnitude = gem::length(pointToLine);
			if (magnitude >= farthestDistance)
			{
				farthestPoint = point;
				farthestDistance = magnitude;
			}
		}

		return farthestPoint;
	}

	std::pair<Polygon, Polygon> SplitPolygon(const Polygon& polygon, Ref<Point> startLine, Ref<Point> endLine)
	{
		std::pair<Polygon, Polygon> result;

		result.first = polygon;
		result.second = polygon.ClonePolygon();

		auto startLine2 = result.second.GetPointFromIndex(startLine->index);
		auto endLine2 = result.second.GetPointFromIndex(endLine->index);

		while (startLine->next->index != endLine->index)
		{
			result.first.RemovePointWithIndex(startLine->next->index);
		}

		while (startLine2->previous->index != endLine2->index)
		{
			result.second.RemovePointWithIndex(startLine2->previous->index);
		}

		return result;
	}

	std::vector<MeshIndex> TriangulatePolygon(const Polygon& polygon)
	{
		std::vector<MeshIndex> indices;

		if (!ValidatePolygon(polygon)) { return indices; }

		for (const auto& p : polygon.points)
		{
			// Check that polygon has enough verts to make triangle
			if (polygon.GetVertexCount() == 3)
			{
				// Successful triangle, add it to indices
				indices.push_back(p->previous->index);
				indices.push_back(p->index);
				indices.push_back(p->next->index);
				break;
			}
			else if (IsConvex(p)) // If first vertex is convex
			{
				auto pointsInTri = GetPointsInTriangle(polygon, p, p->next, p->previous);
				if (pointsInTri.size() == 0)
				{
					// Successful triangle, add it to indices
					indices.push_back(p->previous->index);
					indices.push_back(p->index);
					indices.push_back(p->next->index);

					Polygon newPolygon = polygon;

					// remove added triangle and repeat for the rest
					newPolygon.RemovePointWithIndex(p->index);

					auto result = TriangulatePolygon(newPolygon);
					indices.insert(indices.end(), result.begin(), result.end());
				}
				else
				{
					// Split lists and rerun triangulate
					auto splitPolygons =
						SplitPolygon(polygon, p, GetFarthestPointFromLine(pointsInTri, p->next, p->previous));

					if (ValidatePolygon(splitPolygons.first) && ValidatePolygon(splitPolygons.second))
					{
						auto result = TriangulatePolygon(splitPolygons.first);
						auto result2 = TriangulatePolygon(splitPolygons.second);

						// remove dublicates after this if needed
						indices.insert(indices.end(), result.begin(), result.end());
						indices.insert(indices.end(), result2.begin(), result2.end());
					}
				}
				break;
			}
		}

		return indices;
	}

	bool ValidatePolygon(const Polygon& polygon)
	{
		auto startIndex = polygon.points[0]->index;
		auto current = polygon.points[0]->next;
		for (uint32_t i = 0; i < polygon.points.size(); i++)
		{
			if (i == polygon.points.size() - 1 && current->index == startIndex)
			{
				return true;
			}
			else if (current->index == startIndex)
			{
				break;
			}
			current = current->next;
		}
		return false;
	}

	struct Triangle2
	{
		bool operator==(const Triangle2& rhs)
		{
			return (
				indices[0] == rhs.indices[0] &&
				indices[1] == rhs.indices[1] &&
				indices[2] == rhs.indices[2]);
		}

		std::array<uint32_t, 3> indices;
	};

	Polygon CreatePolygon(const MeshInfo& meshInfo)
	{
		Polygon poly;

		const auto& verts = meshInfo.first;
		const auto& indices = meshInfo.second;

		if (verts.empty() || indices.empty()) { return poly; }

		// Convert indices to triangles
		std::vector<Triangle2> triangles;

		for (uint32_t i = 0; i < indices.size(); i += 3)
		{
			Triangle2 tri;
			tri.indices[0] = indices[i];
			tri.indices[1] = indices[i + 1];
			tri.indices[2] = indices[i + 2];
			triangles.push_back(tri);
		}

		// Check how many triangle each edge is connected to
		std::unordered_map<uint64_t, uint32_t> edgeTriCount;
		for (uint32_t i = 0; i < triangles.size(); i++)
		{
			for (uint32_t j = 0; j < 3; j++)
			{
				uint32_t nextIndex = (j + 1 < 3) ? j + 1 : 0;
				uint64_t edge = ((uint64_t)(triangles[i].indices[j]) << 32) | triangles[i].indices[nextIndex];
				uint64_t edgeFlipped = ((uint64_t)(triangles[i].indices[nextIndex]) << 32) | triangles[i].indices[j];

				if (edgeTriCount.find(edgeFlipped) != edgeTriCount.end())
				{
					edgeTriCount[edgeFlipped] += 1;
				}
				else
				{
					edgeTriCount[edge] += 1;
				}
			}
		}

		std::vector<std::pair<MeshIndex, MeshIndex>> outerEdges;

		// Add edge points to polygon
		for (const auto& edge : edgeTriCount)
		{
			auto edgeKey = edge.first;
			if (edge.second == 1)
			{
				auto edgeStartIndex = (uint32_t)(edgeKey >> 32);
				auto edgeEndIndex = (uint32_t)(edgeKey);

				outerEdges.push_back(std::pair<MeshIndex, MeshIndex>(edgeStartIndex, edgeEndIndex));
				Ref<Point> p = CreateRef<Point>(edgeStartIndex, verts[edgeStartIndex].position);
				poly.points.emplace_back(p);
			}
		}

		// Set point relationships
		for (uint32_t i = 0; i < poly.points.size(); i++)
		{
			auto currentIndex = poly.points[i]->index;
			for (uint32_t j = 0; j < outerEdges.size(); j++)
			{
				auto startIndex = outerEdges[j].first;
				auto endIndex = outerEdges[j].second;
				if (startIndex == currentIndex)
				{
					poly.points[i]->next = poly.GetPointFromIndex(endIndex);
				}
				else if (endIndex == currentIndex)
				{
					poly.points[i]->previous = poly.GetPointFromIndex(startIndex);
				}
			}
		}

		return poly;
	}

	bool MergePolygons(const Polygon& a, const Polygon& b, Polygon& outPoly)
	{
		Polygon result = a.ClonePolygon();
		Polygon copy = b.ClonePolygon();

		copy.ForEach([&](Ref<Point> p)
			{
				p->index += (uint32_t)result.points.size();
			});

		bool successfulMerge = false;

		result.ForEach([&](Ref<Point> point)
			{
				if (!point || !point->next || !point->previous) { return; }
		Line pointLine(gem::vec2(point->position.x, point->position.z), gem::vec2(point->next->position.x, point->next->position.z));

		copy.ForEach([&](Ref<Point> point2)
			{
				if (point == point2) { return; }
		if (!point2 || !point2->next || !point2->previous) { return; }

		Line point2Line(gem::vec2(point2->position.x, point2->position.z), gem::vec2(point2->next->position.x, point2->next->position.z));

		float mergeDistance = 25.f;
		auto distance = gem::dot(pointLine.GetNormal(), point2Line.GetMiddle() - pointLine.GetMiddle());

		if (pointLine.Facing(point2Line) && distance < mergeDistance)
		{
			point->next->previous = point2;
			point2->next->previous = point;

			auto pN = point->next;
			point->next = point2->next;
			point2->next = pN;

			successfulMerge = true;
		}
			});
			});

		if (successfulMerge)
		{
			result.points.insert(result.points.end(), copy.points.begin(), copy.points.end());
			outPoly = result;
		}

		return successfulMerge;
	}

	Polygon MergePolygons(const std::vector<Polygon>& polygons)
	{
		auto polygonList = polygons;
		Polygon result = polygonList[0];

		bool updateResult = true;
		std::vector<uint32_t> mergeIndices;
		mergeIndices.emplace_back(0);

		// Update Indices
		int offset = 0;
		for (auto& poly : polygonList)
		{
			poly.ForEach([&](Ref<Point> p)
				{
					p->index += offset;
				});
			offset += (int32_t)poly.points.size();
		}

		while (updateResult)
		{
			updateResult = false;

			for (uint32_t i = 0; i < polygonList.size(); i++)
			{
				if (std::find(mergeIndices.begin(), mergeIndices.end(), i) != mergeIndices.end()) { continue; }

				if (MergePolygons(result, polygonList[i], result))
				{
					mergeIndices.emplace_back(i);
					updateResult = true;
				}
			}
		}

		return result;
	}

//	std::vector<Ref<Triangle>> CalculateTriangles(const MeshInfo& meshInfo, std::vector<ConnectingSide>& outConnectingSides)
//	{
//		const auto& vertices = meshInfo.first;
//		const auto& indices = meshInfo.second;
//
//		std::vector<Ref<Triangle>> result;
//		std::vector<ConnectingSide> connectingSides;
//
//		for (size_t i = 0; i < indices.size(); i += 3)
//		{
//			Ref<Triangle> node = CreateRef<Triangle>();
//			for (size_t j = 0; j < 3; j++)
//			{
//				uint32_t first = indices[i + j];
//				node->indices[j] = first;
//			}
//
//			const float x = (vertices[node->indices[0]].position.x + vertices[node->indices[1]].position.x + vertices[node->indices[2]].position.x) / 3.f;
//			const float y = (vertices[node->indices[0]].position.y + vertices[node->indices[1]].position.y + vertices[node->indices[2]].position.y) / 3.f;
//			const float z = (vertices[node->indices[0]].position.z + vertices[node->indices[1]].position.z + vertices[node->indices[2]].position.z) / 3.f;
//
//			node->center = { x, y, z };
//			node->index = (uint32_t)result.size();
//			result.emplace_back(node);
//		}
//
//		for (size_t i = 0; i < result.size(); i++)
//		{
//			for (size_t j = 0; j < result.size(); j++)
//			{
//				if (result[i] != result[j])
//				{
//					bool checked = false;
//					for (size_t h = 0; h < result[j]->adjecents.size(); h++)
//					{
//						if (result[j]->adjecents[h] == result[i])
//						{
//							checked = true;
//							break;
//						}
//					}
//					if (!checked)
//					{
//						for (size_t h = 0; h < 3; h++)
//						{
//							size_t secondH = h + 1;
//							size_t pastH = h - 1;
//							if (h == 2)
//							{
//								secondH = 0;
//							}
//							if (h == 0)
//							{
//								pastH = 2;
//							}
//							for (size_t k = 0; k < 3; k++)
//							{
//								size_t secondK = k + 1;
//								size_t pastK = k - 1;
//
//								if (k == 2)
//								{
//									secondK = 0;
//								}
//								if (k == 0)
//								{
//									pastK = 2;
//								}
//
//								if (vertices[result[i]->indices[h]].ComparePosition(vertices[result[j]->indices[k]])) // cursed i hate it
//								{
//									bool hasPassed = false;
//									uint32_t toIndex = 0;
//									uint32_t fromIndex = result[i]->indices[h];
//
//									if (vertices[result[i]->indices[secondH]].ComparePosition(vertices[result[j]->indices[secondK]]))
//									{
//										toIndex = result[i]->indices[secondH];
//										hasPassed = true;
//									}
//									if (vertices[result[i]->indices[pastH]].ComparePosition(vertices[result[j]->indices[pastK]]))
//									{
//										toIndex = result[i]->indices[pastH];
//
//										hasPassed = true;
//
//									}
//									if (vertices[result[i]->indices[secondH]].ComparePosition(vertices[result[j]->indices[pastK]]))
//									{
//										toIndex = result[i]->indices[secondH];
//
//										hasPassed = true;
//
//									}
//									if (vertices[result[i]->indices[pastH]].ComparePosition(vertices[result[j]->indices[secondK]]))
//									{
//										toIndex = result[i]->indices[pastH];
//
//										hasPassed = true;
//
//									}
//
//									if (std::find_if(connectingSides.begin(), connectingSides.end(), [&](const ConnectingSide& side)
//										{
//											return side.fromIndex == fromIndex && side.toIndex == toIndex || side.fromIndex == toIndex && side.toIndex == fromIndex;
//										}) != connectingSides.end())
//									{
//										break;
//									}
//
//										if (hasPassed)
//										{
//											result[i]->adjecents.push_back(result[j]);
//											result[j]->adjecents.push_back(result[i]);
//											ConnectingSide side;
//											side.from = vertices[fromIndex].position;
//											side.to = vertices[toIndex].position;
//											side.fromIndex = fromIndex;
//											side.toIndex = toIndex;
//											connectingSides.push_back(side);
//											result[i]->connections.push_back(side);
//											result[j]->connections.push_back(side);
//											break;
//										}
//								}
//
//							}
//						}
//					}
//				}
//			}
//		}
//
//		return result;
//	}
}
