#pragma once

#include <Volt/Core/Base.h>
#include <Volt/Rendering/Framebuffer.h>
#include <NodeEditor/Node.h>

class FramebufferSplitNode : public NE::Node
{
public:
	FramebufferSplitNode();

	void OnCreate() override;
	void OnLinked(const NE::Pin* pin, const void* userData) override;

private:
	void Rebuild();

	Volt::FramebufferSpecification* myCurrentSpecification = nullptr;
};