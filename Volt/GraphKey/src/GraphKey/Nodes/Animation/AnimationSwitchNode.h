#pragma once

#include "GraphKey/Node.h"

#include <Volt/Asset/Animation/Animation.h>

namespace Volt
{
	class Animation;
}

namespace GraphKey
{
	struct AnimationSwitchNode : public Node
	{
		AnimationSwitchNode();
		~AnimationSwitchNode() override = default;

		inline const std::string GetName() override { return "Animation Switch Node"; }
		inline const gem::vec4 GetColor() override { return { 1.f }; }

	private:
		void GetSwitchedAnimation();

		int32_t myLastFrame = -1;
		Volt::Animation::TRS myLastRootTransform;
	};
}
