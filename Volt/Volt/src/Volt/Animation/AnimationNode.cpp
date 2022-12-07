#include "vtpch.h"
#include "AnimationNode.h"

Volt::AnimationNode::AnimationNode(std::string aName, int aNodeID, int aAnimationIndex)
{
	animationName = aName;
	nodeID = aNodeID;
	currentAnimation = aAnimationIndex;
}

void Volt::AnimationNode::AddLink(const AnimationLink& link)
{
	myLinks.emplace_back(link);
}

const int Volt::AnimationNode::GetAnimationIndex()
{
	return currentAnimation;
}

const int Volt::AnimationNode::GetNodeID()
{
	return nodeID;
}
