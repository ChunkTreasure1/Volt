#include "gkpch.h"
#include "PhysicsNodes.h"

namespace GraphKey
{
	OnCollisionEnterNode::OnCollisionEnterNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr),
			AttributeConfig<Volt::Entity>("Other Entity", AttributeDirection::Output, true)
		};
	}

	void OnCollisionEnterNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<OnCollisionEnterEvent>([this](OnCollisionEnterEvent& e) {
			SetOutputData(1, e.GetOther());
			ActivateOutput(0); return false;
		});
	}

	OnCollisionExitNode::OnCollisionExitNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr),
			AttributeConfig<Volt::Entity>("Other Entity", AttributeDirection::Output, true)
		};
	}

	void OnCollisionExitNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<OnCollisionExitEvent>([this](OnCollisionExitEvent& e) { SetOutputData(1, e.GetOther()); ActivateOutput(0); return false; });
	}

	OnTriggerExitNode::OnTriggerExitNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr),
			AttributeConfig<Volt::Entity>("Other Entity", AttributeDirection::Output, true)
		};
	}

	void OnTriggerExitNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<OnTriggerExitEvent>([this](OnTriggerExitEvent& e) { SetOutputData(1, e.GetOther()); ActivateOutput(0); return false; });
	}

	OnTriggerEnterNode::OnTriggerEnterNode()
	{
		outputs =
		{
			AttributeConfig("", AttributeDirection::Output, nullptr),
			AttributeConfig<Volt::Entity>("Other Entity", AttributeDirection::Output, true)
		};
	}

	void OnTriggerEnterNode::OnEvent(Volt::Event& e)
	{
		Volt::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<OnTriggerEnterEvent>([this](OnTriggerEnterEvent& e) {
			SetOutputData(1, e.GetOther());
			ActivateOutput(0); return false; });
	}
}