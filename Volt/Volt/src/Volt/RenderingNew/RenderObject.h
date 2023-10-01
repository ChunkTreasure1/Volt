#pragma once

#include "Volt/Core/UUID.h"

#include <entt.hpp>

namespace Volt
{
	class Mesh;

	struct RenderObject
	{
		UUID id;
		entt::entity entity;
	
		Ref<Mesh> mesh;
		uint32_t subMeshIndex = 0;
		uint32_t vertexBufferIndex = 0;
	};
}
