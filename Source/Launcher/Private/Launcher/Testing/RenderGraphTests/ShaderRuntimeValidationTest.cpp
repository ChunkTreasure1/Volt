#include "Testing/RenderGraphTests/ShaderRuntimeValidationTest.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/Shader/ShaderMap.h>

using namespace Volt;

RG_ShaderRuntimeValidationTest::RG_ShaderRuntimeValidationTest()
{
}

RG_ShaderRuntimeValidationTest::~RG_ShaderRuntimeValidationTest()
{
}

bool RG_ShaderRuntimeValidationTest::RunTest()
{
	RenderGraph renderGraph{ m_commandBuffer };

	struct Data
	{
		RenderGraphBufferHandle bufferHandle;
	};

	renderGraph.AddPass<Data>("Compute Shader Pass",
	[&](RenderGraph::Builder& builder, Data& data)
	{
		{
			const auto desc = RGUtils::CreateBufferDescGPU<glm::uvec2>(32, "Buffer");
			data.bufferHandle = builder.CreateBuffer(desc);
		}

		builder.SetHasSideEffect();
		builder.SetIsComputePass();
	},
	[=](const Data& data, RenderContext& context)
	{
		auto pipeline = ShaderMap::GetComputePipeline("RG_DispatchComputeShaderTest");

		context.BindPipeline(pipeline);

		context.SetConstant("outputBuffer"_sh, data.bufferHandle);
		context.SetConstant("initialValue"_sh, 1u);

		context.Dispatch(1, 1, 1);

	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
