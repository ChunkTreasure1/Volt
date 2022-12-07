#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	class AnimatedCharacter;
	class AnimationState
	{
	public:
		struct Result
		{
			bool shouldBlend = true;
			float blendSpeed = 1.f;
			int32_t nextState = -1;
		};

		AnimationState(const std::string& name, uint32_t animationIndex, Ref<AnimatedCharacter> character);

		void OnEnter(float startTime);
		void OnExit();

		bool Update(Result& outTargetState);
		void AddTransition(std::function<int32_t()>&& func, bool blendTo = true, float blendSpeed = 1.f);

		inline const uint32_t GetAnimationIndex() const { return myAnimationIndex; }
		inline const float GetCurrentStartTime() const { return myCurrentStartTime; }
		inline const bool ShouldLoop() const { return myShouldLoop; }

		inline const std::string& GetName() const { return myName; }

	private:
		struct AnimationTransition
		{
			bool blendToNext = true;
			float blendSpeed = 1.f;
			std::function<int32_t()> func = nullptr;
		};

		std::string myName;

		uint32_t myAnimationIndex = 0;
		float myCurrentStartTime = 0.f;
		bool myShouldLoop = true;

		Ref<AnimatedCharacter> myCharacter;

		std::vector<AnimationTransition> myTransitions;
	};
}