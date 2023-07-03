#include "gkpch.h"
#include "AudioNodes.h"

GraphKey::PlayAudioNode::PlayAudioNode()
{
	inputs =
	{
		AttributeConfig("Play", AttributeDirection::Input, GK_BIND_FUNCTION(PlayAudioNode::Play)),
		AttributeConfig<Volt::Entity>("Entity with a AudioSource", AttributeDirection::Input),
		AttributeConfig<std::string>("Event", AttributeDirection::Input, false),
		AttributeConfig<std::string>("Parameter", AttributeDirection::Input, false),
		AttributeConfig<float>("Parameter Value", AttributeDirection::Input)
	};

	outputs =
	{
		AttributeConfig("", AttributeDirection::Output),
		AttributeConfig<int>("Instance ID", AttributeDirection::Output, true)
	};
}

const std::string GraphKey::PlayAudioNode::GetName()
{
	return "Play Audio";
}

const glm::vec4 GraphKey::PlayAudioNode::GetColor()
{
	return { 0.f, 0.f, 1.f, 1.f };
}

void GraphKey::PlayAudioNode::Play()
{
	Volt::Entity entity = GetInput<Volt::Entity>(1);
	bool hasComponent = false;
	if (entity.HasComponent<Volt::AudioSourceComponent>())
	{
		hasComponent = true;
	}

	if (hasComponent)
	{
		const auto eventName = GetInput<std::string>(2);
		const auto parameterName = GetInput<std::string>(3);
	}
	ActivateOutput(0);
}


