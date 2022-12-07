#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class Mesh;
	class Shape
	{
	public:
		static Ref<Mesh> CreateUnitCube();
	};
}