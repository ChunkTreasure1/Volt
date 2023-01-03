#pragma once
#include "vector/vector.h"
#include "geometric.h"

namespace Pathfinder
{
	template<typename T>
	class Line3;

	typedef Line3<float> Line3f;

	template<typename T>
	class Line3
	{
	public:
		Line3() = default;
		Line3(const vec<3, T>& s, const vec<3, T>& e) { start = s, end = e; };

		vec<3, T> GetCenter() const { return (start + end) / 2.f; };

		vec<3, T> start;
		vec<3, T> end;
	};
}