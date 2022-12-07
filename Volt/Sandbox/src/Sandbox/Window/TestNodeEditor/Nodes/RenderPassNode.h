#pragma once

#include <Volt/Core/Base.h>
#include <NodeEditor/Node.h>

namespace Volt
{
	class RenderPipeline;
}

class RenderPassNode : public NE::Node
{
public:
	RenderPassNode(Ref<Volt::RenderPipeline> renderPipeline, uint32_t passIndex);

	void OnCreate() override;
	void DrawContent() override;

private:
	Ref<Volt::RenderPipeline> myRenderPipeline;
	uint32_t myPassIndex;
};