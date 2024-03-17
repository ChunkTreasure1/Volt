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

	inline bool operator==(const RenderObject& lhs, const RenderObject& rhs) { return lhs.id == rhs.id; }
	inline bool operator!=(const RenderObject& lhs, const RenderObject& rhs) { return !(lhs == rhs); }

	inline bool operator==(const RenderObject& lhs, const UUID64& rhs) { return lhs.id == rhs; }
	inline bool operator!=(const RenderObject& lhs, const UUID64& rhs) { return !(lhs == rhs); }
}
