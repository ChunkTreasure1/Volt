#pragma once

#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Scene/Entity.h"

#include <unordered_map>
#include <any>

namespace Volt
{
	class AnimatedCharacter;
	class AnimationGraphAsset;
	class Event;
	class AnimationController
	{
	public:
		AnimationController(Ref<Volt::AnimationGraphAsset> graph);
		AnimationController(Ref<Volt::AnimationGraphAsset> graph, Entity entity);

		const Vector<glm::mat4> Sample();

		inline Ref<Volt::AnimationGraphAsset> GetGraph() const { return myGraph; }
		inline const auto& GetRootMotion() const { return myRootMotion; }

		void AttachEntity(const std::string& attachment, Entity entity);
		void DetachEntity(Entity entity);

		void OnEvent(Event& e);

	private:
		Animation::TRS myLastHipTransform;
		Animation::TRS myLastRootTransform;
		Animation::TRS myRootMotion;

		Volt::Entity myEntity;

		Ref<Volt::AnimationGraphAsset> myGraph;

		float myCurrentStartTime = 0.f;

		std::unordered_map<UUID64, Vector<Entity>> myAttachedEntities;
	};
}
