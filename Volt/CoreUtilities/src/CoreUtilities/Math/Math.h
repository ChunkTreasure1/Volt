#pragma once

#include "CoreUtilities/CompilerTraits.h"
#include "CoreUtilities/Concepts.h"

#include <type_traits>

namespace Math
{
	template<Integer T>
	VT_NODISCARD VT_INLINE T Get1DIndexFrom3DCoord(T x, T y, T z, T maxX, T maxY)
	{
		return (z * maxX * maxY) + (y * maxX) + x;
	}

	template<Integer T>
	VT_NODISCARD VT_INLINE std::array<T, 3> Get3DCoordFrom1DIndex(T index, T maxX, T maxY)
	{
		T z = index / (maxX * maxY);
		index -= (z * maxX * maxY);
		T y = index / maxX;
		T x = index % maxX;

		return { x, y, z };
	}

	VT_NODISCARD VT_INLINE glm::vec3 DirectionToVector(const glm::vec2& direction)
	{
		glm::vec3 result;
		result.x = glm::sin(direction.y) * glm::cos(direction.x);
		result.y = -glm::cos(direction.y);
		result.z = glm::sin(direction.y) * glm::sin(direction.x);

		return result;
	}
}
