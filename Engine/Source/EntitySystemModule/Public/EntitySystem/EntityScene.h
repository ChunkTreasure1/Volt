#pragma once

#include "EntitySystem/EntityTransformCache.h"
#include "EntitySystem/EntityRegistry.h"

#include <entt.hpp>

class ECSBuilder;
class ScriptingEngine;

namespace Volt
{
	class RenderScene;

	class VTES_API EntityScene
	{
	public:
		EntityScene();
		~EntityScene();

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void Update(float deltaTime);
		void FixedUpdate(float deltaTime);

		void SortScene();
		void ClearScene();

		EntityHelper CreateEntity(const std::string& tag = "");
		EntityHelper CreateEntityWithID(EntityID id, const std::string& tag = "");

		void DestroyEntity(EntityID id, bool isDestroyingChildFromParent = false);

		void MarkEntityAsEdited(const EntityHelper& entityHelper);
		void ClearEditedEntities();

		Vector<EntityID> InvalidateEntityTransform(EntityID entityId);

		VT_NODISCARD bool IsEntityValid(EntityID entityId) const;
		VT_NODISCARD TQS GetEntityWorldTQS(const EntityHelper& entityHelper) const;
		VT_NODISCARD EntityHelper GetEntityHelperFromEntityID(EntityID entityId) const;
		VT_NODISCARD EntityHelper GetEntityHelperFromEntityHandle(entt::entity entityHandle) const;
		VT_NODISCARD uint32_t GetEntityAliveCount() const;

		VT_NODISCARD const std::set<EntityID>& GetEditedEntities() const { return m_entityRegistry.GetEditedEntities(); }
		VT_NODISCARD const std::set<EntityID>& GetRemovedEntities() const { return m_entityRegistry.GetRemovedEntities(); }

		VT_NODISCARD VT_INLINE entt::registry& GetRegistry() { return m_registry; }
		VT_NODISCARD VT_INLINE const entt::registry& GetRegistry() const { return m_registry; }

		// #TODO_Ivar: Hack until we can figure out a proper structure
		VT_NODISCARD VT_INLINE RenderScene* GetRenderScene() const { return m_renderScene; }
		VT_INLINE void SetRenderScene(RenderScene* renderScene) { m_renderScene = renderScene; }

	private:
		friend class EntityHelper;

		void Initialize();

		void ComponentOnStart();
		void ComponentOnStop();

		entt::registry m_registry;

		bool m_isPlaying = false;

		Scope<ECSBuilder> m_ecsBuilder;
		Scope<ScriptingEngine> m_scriptingEngine;
		EntityRegistry m_entityRegistry;
		mutable EntityTransformCache m_transformCache;

		RenderScene* m_renderScene;
	};
}
