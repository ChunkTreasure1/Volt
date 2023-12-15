#pragma once

#include <entt.hpp>

namespace Volt
{
	class Scene;
	class AnimationStateMachine;
	class AnimationSystem
	{
	public:
		AnimationSystem(Scene* scene);

		void OnRuntimeStart(entt::registry& registry);
		void OnRuntimeEnd(entt::registry& registry);
		void Update(entt::registry& registry, float deltaTime);

	private:
		Scene* myScene;
	};
}
