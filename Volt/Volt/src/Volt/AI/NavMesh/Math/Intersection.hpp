#pragma once
#include "Plane.hpp"
#include "Ray.hpp"
#include "Line.hpp"

#include <gem/gem.h>
#include <math.h>

namespace Volt
{
	// If the ray is parallel to the plane, aOutIntersectionPoint remains unchanged. If
	// the ray is in the plane, true is returned, if not, false is returned. If the ray
	// isn't parallel to the plane, the intersection point is stored in
	// aOutIntersectionPoint and true returned.
	template<typename T>
	bool IntersectionPlaneRay(const Plane<T>& aPlane, const Ray<T>& aRay, gem::vec<3, T>&
		aOutIntersectionPoint);
	template<typename T>
	gem::vec<2, T> IntersectionLine(Volt::Line<T> l1, Volt::Line<T> l2);
	template<typename T>
	bool IsIntersectionLine(Volt::Line<T> l1, Volt::Line<T> l2);
}

template<typename T>
bool Volt::IntersectionPlaneRay(const Plane<T>& aPlane, const Ray<T>& aRay, gem::vec<3, T>& aOutIntersectionPoint)
{
	Plane<T> plane = aPlane;
	Ray<T> ray = aRay;

	T denom = plane.GetNormal().Dot(ray.GetDirection());
	gem::vec<3, T> distance = plane.GetPosition() - ray.GetOrigin();
	T pointLength;

	if (denom == 0)
	{
		return (distance.Dot(plane.GetNormal()) == 0);
	}
	else if (std::abs(denom) > std::numeric_limits<T>::epsilon())
	{
		pointLength = distance.Dot(plane.GetNormal()) / denom;

		aOutIntersectionPoint = ray.GetOrigin() + ray.GetDirection().GetNormalized() * pointLength;

		return (pointLength >= 0);
	}

	return false;
}

template<typename T>
gem::vec<2, T> Volt::IntersectionLine(Volt::Line<T> l1, Volt::Line<T> l2)
{
	T a = l2.GetEnd().y - l2.GetStart().y;
	T b = l2.GetStart().x - l2.GetEnd().x;
	T c = l2.GetEnd().x * l2.GetStart().y - l2.GetStart().x * l2.GetEnd().y;
	T u = std::fabs(a * l1.GetStart().x + b * l1.GetStart().y + c);
	T v = std::fabs(a * l1.GetEnd().x + b * l1.GetEnd().y + c);
	return gem::vec<2, T>((l1.GetStart().x * v + l1.GetEnd().x * u) / (u + v), (l1.GetStart().y * v + l1.GetEnd().y * u) / (u + v));
}

template<typename T>
bool Volt::IsIntersectionLine(Volt::Line<T> l1, Volt::Line<T> l2)
{
	auto result = IntersectionLine(l1, l2);
	if (l1.On(result) && l2.On(result))
	{
		return true;
	}
	return false;
}