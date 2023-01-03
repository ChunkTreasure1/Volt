#pragma once

#include "Volt/Core/Base.h"

#include <gem/gem.h>
#include <any>

namespace Volt
{
	class AnimationState;
	class AnimatedCharacter;
	class AnimationStateMachine
	{
	public:
		AnimationStateMachine(Ref<AnimatedCharacter> character);
		void SetStartSate(uint32_t index);
		void SetCurrentState(uint32_t index);

		Ref<AnimationState> AddState(const std::string& name, uint32_t animationIndex);
		
		void Update();
		const std::vector<gem::mat4> Sample();

		inline const Ref<AnimatedCharacter> GetCharacter() const { return myCharacter; }
		inline std::unordered_map<std::string, std::any>& GetBlackboard() { return myBlackboard; }
		inline const size_t GetStateCount() const { return myStates.size(); }

	private:
		AnimationState* myCurrentState = nullptr;
		std::vector<Ref<AnimationState>> myStates;
		std::unordered_map<std::string, std::any> myBlackboard;

		bool myIsCrossfading = false;
		float myCrossfadeFromStartTime = 0.f;
		float myCrossfadeToStartTime = 0.f;
		float myCrossfadeTime = 0.f;
		float myCrossfadeSpeed = 1.f;
		uint32_t myCrossfadeFrom = 0;

		Ref<AnimatedCharacter> myCharacter;
	};
}