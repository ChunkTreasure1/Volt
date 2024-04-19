#include "vtpch.h"
#include "PrefixSumTechnique.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"

#include "Volt/Math/Math.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt
{
	PrefixSumTechnique::PrefixSumTechnique(RenderGraph& rg)
		: m_renderGraph(rg)
	{
	}

	void PrefixSumTechnique::Execute(RenderGraphResourceHandle inputBuffer, RenderGraphResourceHandle outputBuffer, const uint32_t valueCount)
	{
		constexpr uint32_t TG_SIZE = 512;

		struct PrefixSumData
		{
			RenderGraphResourceHandle stateBuffer;
		};

		struct State
		{
			uint32_t aggregate;
			uint32_t prefix;
			uint32_t state;
		};

		const uint32_t groupCount = Math::DivideRoundUp(valueCount, TG_SIZE);

		auto pipeline = ShaderMap::GetComputePipeline("PrefixSum");

		m_renderGraph.AddPass<PrefixSumData>("Prefix Sum",
		[&](RenderGraph::Builder& builder, PrefixSumData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<State>(std::max(groupCount, 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "State Buffer");
				data.stateBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(inputBuffer);
			builder.WriteResource(outputBuffer);
			builder.SetIsComputePass();
		},
		[pipeline, groupCount, inputBuffer, outputBuffer, valueCount](const PrefixSumData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			context.ClearBuffer(data.stateBuffer, 0);

			context.BindPipeline(pipeline);
			context.SetConstant("inputValues"_sh, resources.GetBuffer(inputBuffer));
			context.SetConstant("outputValues"_sh, resources.GetBuffer(outputBuffer));
			context.SetConstant("stateBuffer"_sh, resources.GetBuffer(data.stateBuffer));
			context.SetConstant("valueCount"_sh, valueCount);

			context.Dispatch(groupCount, 1, 1);
		});
	}
}
