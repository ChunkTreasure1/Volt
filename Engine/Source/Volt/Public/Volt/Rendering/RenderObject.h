#pragma once

#include <CoreUtilities/UUID.h>
#include <EntitySystem/EntityID.h>

namespace Volt
{
	class Mesh;
	class MotionWeaver;

	using RenderObjectID = UUID64;

	struct RenderObject
	{
		RenderObjectID id;
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

	inline bool operator==(const RenderObject& lhs, const RenderObjectID& rhs) { return lhs.id == rhs; }
	inline bool operator!=(const RenderObject& lhs, const RenderObjectID& rhs) { return !(lhs == rhs); }
}
