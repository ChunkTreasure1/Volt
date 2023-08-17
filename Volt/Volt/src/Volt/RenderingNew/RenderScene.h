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

		void SetValid();
		void Invalidate();

		const UUID Register(Wire::EntityId entityId, Ref<Mesh> mesh, uint32_t subMeshIndex);
		void Unregister(UUID id);

		inline const bool IsInvalid() const { return m_isInvalid; }
		inline const std::vector<RenderObject>& GetObjects() const { return m_renderObjects; }

		std::vector<RenderObject>::iterator begin() { return m_renderObjects.begin(); }
		std::vector<RenderObject>::iterator end() { return m_renderObjects.end(); }

		const std::vector<RenderObject>::const_iterator cbegin() const { return m_renderObjects.cbegin(); }
		const std::vector<RenderObject>::const_iterator cend() const { return m_renderObjects.cend(); }

	private:
		std::vector<RenderObject> m_renderObjects;
		Scene* m_scene = nullptr;

		bool m_isInvalid = false;
	};
}
