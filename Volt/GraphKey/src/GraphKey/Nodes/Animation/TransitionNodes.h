#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	struct TransitionOutputNode : public Node
	{
		TransitionOutputNode();
		inline ~TransitionOutputNode() override = default;

		const bool Evaluate();

		inline const std::string GetName() override { return "Result"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }
	};
}