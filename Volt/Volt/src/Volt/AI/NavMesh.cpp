#include "vtpch.h"
#include "NavMesh.h"

namespace Volt
{
	Pathfinder::vec3 Volt::VTtoPF(gem::vec3 x)
	{
		{ return { x.x, x.y, x.z }; };
	}

	gem::vec3 Volt::PFtoVT(Pathfinder::vec3 x)
	{
		{ return { x.x, x.y, x.z }; }
	}
}