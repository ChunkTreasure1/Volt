#pragma once

#include "GraphKey/Node.h"

#include <Volt/Asset/Animation/Animation.h>

namespace GraphKey
{
	struct AnimationOutputData
	{
		Volt::Animation::TRS rootTRS;
		Vector<Volt::Animation::TRS> pose;
	};

	struct OutputPoseNode : public Node
	{
		OutputPoseNode();
		~OutputPoseNode() override = default;

		const AnimationOutputData Sample(bool moveToWorldSpace, float startTime);

		inline const std::string GetName() override { return "Output Pose"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
	};
}
