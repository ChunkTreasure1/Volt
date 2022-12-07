#pragma once
#include <string>
#include <vector>
#include "Volt/Core/Base.h"

namespace Volt
{
	struct AnimationParameter;
	class AnimationNode;

	enum class PinType
	{
		Input,
		Output
	};

	enum class ComparisonType
	{
		Equal,
		Greater,
		Less
	};

	struct BlendRequirement
	{
		ComparisonType comparisonType = ComparisonType::Equal;
		AnimationParameter* parameter = nullptr;
		float value = 0.f;
	};

	struct AnimationLink
	{
		AnimationNode* ToNode;

		bool hasExitTime = true;
		float blendTime = 0.1f;
		std::vector<BlendRequirement> blendRequirements;
	};

	class AnimationNode
	{
	public:
		AnimationNode() = default;
		AnimationNode(std::string aName,int nodeID ,int aAnimationIndex);
		~AnimationNode() = default;

		void AddLink(const AnimationLink& link);
		//void Enter(float aBlendTime);
		//void Exit();

		const int GetAnimationIndex();
		const int GetNodeID();

		std::string animationName;
		int nodeID = -1;
		int currentAnimation = -1;
		float speed = 1.0f;

		float currentStartTime = 0.f;
		bool isLooping = false;

		float crossfadeStartTime = 0.f;
		bool isCrossFading = false;
		bool shouldCrossfade = false;

		uint32_t crossfadeFrom = 0;
		uint32_t crossfadeTo = 0;

		bool castShadows = true;

		std::vector<AnimationLink> myLinks;

	private:
		//bool EvaluateRequirement(const BlendRequirement& aReq);
	};
}

