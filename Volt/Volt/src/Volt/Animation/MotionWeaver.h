#pragma once

#include "Volt/Asset/Animation/MotionWeaveAsset.h"

#include <glm/glm.hpp>

//this class will be used to control the animation of a character
//It works by having a main animation and a list of all the previous animations, the main animation has a weight that increases to 1 over time, the previous animations have a weight that decreases to 0 over time
//when the weight of a previous animation reaches 0, it is removed from the list
namespace Volt
{
	class Animation;
	class Skeleton;
	
	struct MotionWeaveEntry
	{
		Ref<Animation> animation;
		float weight = 0.f;
		float speed = 1.f;
		float startTime = 0.f;
		bool looping = true;
	};
	class MotionWeaver
	{
	public:
		MotionWeaver(Ref<MotionWeaveAsset> motionWeaveAsset, Ref<Skeleton> skeleton);
		~MotionWeaver();

		void Update(float deltaTime);

		const std::vector<glm::mat4> Sample();

	private:
		Ref<MotionWeaveAsset> m_MotionWeaveAsset;
		Ref<Skeleton> m_Skeleton;

		glm::vec3 m_PrevRootPosition;
		
		std::vector<MotionWeaveEntry> m_Entries;

	};

}
