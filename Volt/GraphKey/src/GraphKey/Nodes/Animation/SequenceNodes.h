#pragma once

#include "GraphKey/Node.h"

#include <Volt/Asset/Animation/Animation.h>

namespace Volt
{
	class Animation;
}

namespace GraphKey
{
	struct SequencePlayerNode : public Node
	{
		SequencePlayerNode();
		~SequencePlayerNode() override = default;

		inline const std::string GetName() override { return "Sequence Player"; }
		inline const gem::vec4 GetColor() override { return { 1.f }; }

		Ref<Volt::Animation> GetAnimation();
		const bool IsLooping();

	private:
		void TrySampleAnimation();
	
		int32_t myLastFrame = -1;
		Volt::Animation::TRS myLastRootTransform;
	};
}