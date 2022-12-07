#pragma once

namespace Volt
{
	class AnimationManager
	{
	public:
		inline static float globalClock = 0.f;
		
		inline static void Reset()
		{
			globalClock = 0.f;
		}

		inline static void Update(float deltaTime)
		{
			globalClock += deltaTime;
		}

	private:
		AnimationManager() = default;
	};
}