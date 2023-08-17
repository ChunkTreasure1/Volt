#pragma once

#include "Volt/Core/UUID.h"

#include <Wire/Wire.h>

namespace Volt
{
	class Mesh;

	struct RenderObject
	{
		UUID id;
		Wire::EntityId entity;
	
		Ref<Mesh> mesh;
		uint32_t subMeshIndex = 0;
		uint32_t vertexBufferIndex = 0;
	};
}
