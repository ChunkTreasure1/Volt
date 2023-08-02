#pragma once
#include "GraphKey/Node.h"
#include "Volt/Core/Base.h"
namespace Volt
{
	class Animation;
}
namespace GraphKey
{
	class GetRelevantAnimationLengthNode : public Node
	{
	public:
		GetRelevantAnimationLengthNode();
		~GetRelevantAnimationLengthNode() override = default;

		inline const std::string GetName() override { return "Get Relevant Animation Length"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }
	private:
		void GetRelevantAnimationLength();
	};

	//class GetRelevantAnimationTime
}
