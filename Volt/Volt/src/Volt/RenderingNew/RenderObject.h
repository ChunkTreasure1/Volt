#pragma once

#include "Volt/Core/UUID.h"
#include "Volt/Scene/EntityID.h"

#include <entt.hpp>

namespace Volt
{
	class Mesh;

	struct RenderObject
	{
		UUID64 id;
		EntityID entity;
	
		Weak<Mesh> mesh;
		uint32_t subMeshIndex = 0;
		uint32_t vertexBufferIndex = 0;
	};
}
