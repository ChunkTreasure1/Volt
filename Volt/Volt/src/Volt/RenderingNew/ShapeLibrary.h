#pragma once

#include <CoreUtilities/Core.h>

namespace Volt
{
	class Mesh;
	class ShapeLibrary
	{
	public:
		static Ref<Mesh> GetCube();
		static Ref<Mesh> GetSphere();

	private:
		ShapeLibrary() = delete;
	};
}
