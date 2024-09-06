#pragma once

#include "Volt/Core/UUID.h"

#include <EntitySystem/EntityID.h>

#include <entt.hpp>

namespace Volt
{
	class Mesh;
	class MotionWeaver;

	struct RenderObject
	{
		UUID64 id;
		EntityID entity;
	
		Weak<Mesh> mesh;
		Ref<MotionWeaver> motionWeaver;
		Weak<Material> material;

		uint32_t subMeshIndex = 0;
		uint32_t vertexBufferIndex = 0;
		uint32_t meshletStartOffset = 0;

		VT_NODISCARD VT_INLINE bool IsAnimated() const { return motionWeaver != nullptr; }
	};

	inline bool operator==(const RenderObject& lhs, const RenderObject& rhs) { return lhs.id == rhs.id; }
	inline bool operator!=(const RenderObject& lhs, const RenderObject& rhs) { return !(lhs == rhs); }

	inline bool operator==(const RenderObject& lhs, const UUID64& rhs) { return lhs.id == rhs; }
	inline bool operator!=(const RenderObject& lhs, const UUID64& rhs) { return !(lhs == rhs); }
}
