#include "vtpch.h"
#include "RenderPipeline.h"

namespace Volt
{
	const uint32_t RenderPipeline::AddRenderPass()
	{
		const uint32_t index = (uint32_t)myRenderPasses.size();
		myRenderPasses.emplace_back();

		return index;
	}
}