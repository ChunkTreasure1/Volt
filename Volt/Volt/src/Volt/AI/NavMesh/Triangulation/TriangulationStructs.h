#pragma once
#include "Volt/Core/Base.h"
#include "Volt/AI/NavMesh/NavMeshData.h"
#include "Volt/AI/NavMesh/Math/Line.hpp"
#include "Volt/AI/NavMesh/Math/Intersection.hpp"

#include "Volt/Rendering/Renderer.h"

#include <gem/gem.h>
#include <vector>
#include <algorithm>
#include <functional>

namespace Volt
{
	struct Point
	{
		Point() {}
		Point(const MeshIndex& ind, const gem::vec3& pos)
			: position(pos), index(ind) {}

		Point(const MeshIndex& ind, const float& x, const float& y, const float& z)
			: position(gem::vec3(x, y, z)), index(ind) {}

		MeshIndex index = 0;
		gem::vec3 position = { 0.f, 0.f, 0.f };

		Ref<Point> next;
		Ref<Point> previous;
	};

	struct Polygon
	{
		Polygon ClonePolygon() const
		{
			// Create clone with new ptrs
			Polygon clone;

			for (auto& p : points)
			{
				clone.points.emplace_back(CreateRef<Point>(p->index, p->position));
			}

			for (auto& p : clone.points)
			{
				const auto& orgP = GetPointFromIndex(p->index);
				p->next = clone.GetPointFromIndex(orgP->next->index);
				p->previous = clone.GetPointFromIndex(orgP->previous->index);
			}

			return clone;
		}

		Ref<Point> GetPointFromIndex(const MeshIndex& index) const
		{
			for (uint32_t i = 0; i < points.size(); i++) 
			{
				if (points[i]->index == index)
				{
					return points[i];
				}
			}
			return Ref<Point>();
		}

		void RemovePointWithIndex(const MeshIndex& index)
		{
			Ref<Point> p = GetPointFromIndex(index);

			if (p)
			{
				p->next->previous = p->previous;
				p->previous->next = p->next;

				points.erase(std::find(points.begin(), points.end(), p));
			}
		}

		void ForEach(std::function<void(Ref<Point>)> fnc)
		{
			if (!points.empty())
			{
				Ref<Point> currentPoint = points[0];
				for (uint32_t i = 0; i < points.size(); i++)
				{
					if (currentPoint)
					{
						fnc(currentPoint);
						currentPoint = currentPoint->next;
					}
				}
			}
		}

		bool IsInLineOfSight(gem::vec3 a, gem::vec3 b) const
		{
			gem::vec3 direction = gem::normalize(b - a);
			Line<float> line(a + direction * 10.f, b - direction * 10.f);

			auto polys = SplitToSeperate();
			bool result = false;

			for (const auto& poly : polys)
			{
				uint32_t intersectionCount = 0;
				for (uint32_t i = 0; i < poly.points.size(); i++)
				{
					Line<float> l(poly.points[i]->position, poly.points[i]->next->position);
					if (IsIntersectionLine(line, l))
					{
						break;
					}

					//if ((line.GetMiddle().x <= l.GetStart().x || line.GetMiddle().x <= l.GetEnd().x) && l.Inside(line.GetMiddle()))
					if (line.GetMiddle().x <= l.GetStart().x && line.GetMiddle().x <= l.GetEnd().x)
					{
						if ((line.GetMiddle().y <= l.GetStart().y && line.GetMiddle().y >= l.GetEnd().y) ||
							(line.GetMiddle().y >= l.GetStart().y && line.GetMiddle().y <= l.GetEnd().y))
						{
							intersectionCount++;
						}
					}

					if (i == poly.points.size() - 1 && intersectionCount % 2 == 1) { result = true; }
				}
				if (result) { break; }
			}
			return result;
		}

		bool IsSingle() const
		{
			auto startIndex = points.front()->index;
			auto current = points.front()->next;
			int count = 1;
			while (current->index != startIndex)
			{
				current = current->next;
				count++;
			}
			return (points.size() == count);
		}

		std::vector<Polygon> SplitToSeperate() const
		{
			std::vector<Polygon> polygons;

			std::set<uint32_t> usedIndices;

			if (IsSingle())
			{
				polygons.push_back(*this);
			}
			else
			{
				// Seperate polygons into vector
				for (uint32_t i = 0; i < points.size(); i++)
				{
					Polygon newPoly;
					if (!usedIndices.contains(points[i]->index))
					{
						auto startIndex = points[i]->index;
						auto current = points[i];
						do
						{
							newPoly.points.push_back(current);
							
							usedIndices.insert(current->index);
							current = current->next;
						} while (current->index != startIndex);

						polygons.push_back(newPoly);
					}
				}
			}

			return polygons;
		}

		bool IsInside(gem::vec2 point, uint32_t& outPolyIndex) const
		{
			auto polys = SplitToSeperate();

			for (uint32_t i = 0; i < polys.size(); i++)
			{
				uint32_t intersectionCount = 0;

				for (uint32_t j = 0; j < polys[i].points.size(); j++)
				{
					Line<float> l(polys[i].points[j]->position, polys[i].points[j]->next->position);

					//if ((point.x <= l.GetStart().x || point.x <= l.GetEnd().x) && l.Inside(point))
					if (point.x <= l.GetStart().x && point.x <= l.GetEnd().x)
					{
						if ((point.y <= l.GetStart().y && point.y >= l.GetEnd().y) ||
							(point.y >= l.GetStart().y && point.y <= l.GetEnd().y))
						{
							intersectionCount++;
						}
					}
				}

				if (intersectionCount % 2 == 1) 
				{ 
					outPolyIndex = i;
					return true;
				}
			}

			return false;
		}

		bool IsInsideAny(gem::vec2 point) const
		{
			auto polys = SplitToSeperate();

			for (const auto& poly : polys)
			{
				uint32_t intersectionCount = 0;

				for (uint32_t i = 0; i < poly.points.size(); i++)
				{
					Line<float> l(poly.points[i]->position, poly.points[i]->next->position);

					//if ((point.x <= l.GetStart().x || point.x <= l.GetEnd().x) && l.Inside(point))
					if (point.x <= l.GetStart().x && point.x <= l.GetEnd().x)
					{
						if ((point.y <= l.GetStart().y && point.y >= l.GetEnd().y) ||
							(point.y >= l.GetStart().y && point.y <= l.GetEnd().y))
						{
							intersectionCount++;
						}
					}
				}

				if (intersectionCount % 2 == 1)
				{
					return true;
				}
			}

			return false;
		}

		uint32_t GetVertexCount() const { return (uint32_t)points.size(); }

		std::vector<Ref<Point>> points;
	};
}