#pragma once

#include "Volt/RenderingNew/RenderObject.h"

#include <span>

namespace Volt
{
	class Scene;

	class RenderScene
	{
	public:
		RenderScene(Scene* sceneRef);
		~RenderScene() = default;

		void PrepareForUpdate();

		void SetValid();
		void Invalidate();

		const UUID Register(Wire::EntityId entityId, Ref<Mesh> mesh, uint32_t subMeshIndex);
		void Unregister(UUID id);

		inline const bool IsInvalid() const { return m_isInvalid; }
		inline const uint32_t GetRenderObjectCount() const { return static_cast<uint32_t>(m_renderObjects.size()); }
		inline const uint32_t GetIndividualMeshCount() const { return m_currentIndividualMeshCount; }

		inline const std::span<const Weak<Mesh>> GetIndividualMeshes() const { return m_individualMeshes; }

		std::vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		std::vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const std::vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const std::vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

	private:
		std::vector<RenderObject> m_renderObjects;
		std::vector<Weak<Mesh>> m_individualMeshes;

		Scene* m_scene = nullptr;
		uint32_t m_currentIndividualMeshCount = 0;

		bool m_isInvalid = false;
	};
}
