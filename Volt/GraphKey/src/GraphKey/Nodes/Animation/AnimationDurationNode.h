#pragma once

#include "GraphKey/Node.h"

#include <Volt/Asset/Animation/Animation.h>

namespace Volt
{
	class Animation;
}

namespace GraphKey
{
	struct GetAnimationDurationNode : public Node
	{
		GetAnimationDurationNode();
		~GetAnimationDurationNode() override = default;

		inline const std::string GetName() override { return "Get Animation Duration"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

		void GetAnimationDuration();
		void GetAnimationSpeed();
		Ref<Volt::Animation> GetAnimation();
	};
}
