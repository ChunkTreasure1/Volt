#pragma once

namespace Wire
{
	class Registry;
}

namespace Volt
{
	class Scene;
	class AnimationStateMachine;
	class AnimationSystem
	{
	public:
		AnimationSystem(Scene* scene);

		void OnRuntimeStart(Wire::Registry& registry);
		void OnRuntimeEnd(Wire::Registry& registry);
		void Update(Wire::Registry& registry, float deltaTime);

	private:
		Scene* myScene;
	};
}
