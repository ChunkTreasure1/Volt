#include "sbpch.h"
#include "FramebufferSplitNode.h"

FramebufferSplitNode::FramebufferSplitNode()
	: NE::Node("Framebuffer Split")
{}

void FramebufferSplitNode::OnCreate()
{
	Rebuild();
}

void FramebufferSplitNode::OnLinked(const NE::Pin* pin, const void* userData)
{
	switch (pin->type)
	{
		case NE::PinType::Framebuffer:
		{
			myCurrentSpecification = (Volt::FramebufferSpecification*)userData;
			break;
		}
	}

	Rebuild();
}

void FramebufferSplitNode::Rebuild()
{
	if (myInputs.empty())
	{
		AddInput("Input", NE::PinType::Framebuffer);
	}

	if (myCurrentSpecification)
	{
		for (int32_t i = (int32_t)myOutputs.size() - 1; i >= 0; --i)
		{
			if (std::find_if(myCurrentSpecification->attachments.begin(), myCurrentSpecification->attachments.end(), [&](const Volt::FramebufferAttachment& att) 
				{
					return att.debugName == myOutputs.at(i)->name;
				}) == myCurrentSpecification->attachments.end())
			{
				myOutputs.erase(myOutputs.begin() + i);
			}
		}

		for (const auto& att : myCurrentSpecification->attachments)
		{
			if (std::find_if(myOutputs.begin(), myOutputs.end(), [&](std::shared_ptr<NE::Pin> pin) 
				{
					return att.debugName == pin->name;
				}) == myOutputs.end())
			{
				AddOutput(att.debugName, Volt::Utility::IsDepthFormat(att.format) ? NE::PinType::DepthAttachment : NE::PinType::ColorAttachment);
			}
		}
	}
	else
	{
		myOutputs.clear();
	}
}
