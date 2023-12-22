#pragma once
#include "GraphKey/Node.h"
#include "Volt/Core/Base.h"
namespace Volt
{
	class Animation;
}
namespace GraphKey
{
	inline Ref<Node> GetRelevantAnimationNode(GraphKey::Graph* aGraph);
	
	
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

	class GetRelevantAnimationTimeNode : public Node
	{
	public:
		GetRelevantAnimationTimeNode();
		~GetRelevantAnimationTimeNode() override = default;

		inline const std::string GetName() override { return "Get Relevant Animation Time"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }
	private:
		void GetRelevantAnimationTime();
	};
	
	class GetRelevantAnimationTimeNormalizedNode : public Node
	{
	public:
		GetRelevantAnimationTimeNormalizedNode();
		~GetRelevantAnimationTimeNormalizedNode() override = default;

		inline const std::string GetName() override { return "Get Relevant Animation Time Normalized"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }
	private:
		void GetRelevantAnimationTimeNormalized();
	};

}
