#include "sbpch.h"
#include "RenderPassNode.h"

#include <Volt/Rendering/RenderPipeline.h>
#include <Volt/Utility/UIUtility.h>

#include <NodeEditor/Graph.h>

RenderPassNode::RenderPassNode(Ref<Volt::RenderPipeline> renderPipeline, uint32_t passIndex)
	: NE::Node("Render Pass"), myRenderPipeline(renderPipeline), myPassIndex(passIndex)
{
}

void RenderPassNode::OnCreate()
{
	AddInput("Render", NE::PinType::Flow);
	AddInput("Framebuffer", NE::PinType::Framebuffer);
}

void RenderPassNode::DrawContent()
{
	if (ImGui::Button("TestButton"))
	{

	}
}