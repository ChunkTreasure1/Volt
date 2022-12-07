#pragma once

namespace Volt
{
	class Scene;
	class PhysicsSystem
	{
	public:
		PhysicsSystem(Scene* aScene);

		void Update(float aDeltaTime);

	private:
		Scene* myScene;
	};
}