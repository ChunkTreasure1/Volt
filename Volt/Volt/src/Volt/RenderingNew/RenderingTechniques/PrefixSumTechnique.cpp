#include "vtpch.h"
#include "PrefixSumTechnique.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphUtils.h"

#include "Volt/Math/Math.h"

#include <VoltRHI/Buffers/BufferView.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

namespace Volt
{
	PrefixSumTechnique::PrefixSumTechnique(RenderGraph& rg, Ref<RHI::ComputePipeline> prefixSumPipeline)
		: m_renderGraph(rg), m_pipeline(prefixSumPipeline)
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

		m_renderGraph.AddPass<PrefixSumData>("Prefix Sum",
		[&](RenderGraph::Builder& builder, PrefixSumData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<State>(groupCount, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "State Buffer");
				data.stateBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(inputBuffer);
			builder.WriteResource(outputBuffer);
			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[pipeline = m_pipeline, groupCount, inputBuffer, outputBuffer, valueCount](const PrefixSumData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::StorageBuffer> stateBuffer = resources.GetBufferRaw(data.stateBuffer);

			context.ClearBuffer(stateBuffer, 0);

			context.BindPipeline(pipeline);
			context.SetConstant(resources.GetBuffer(inputBuffer));
			context.SetConstant(resources.GetBuffer(outputBuffer));
			context.SetConstant(resources.GetBuffer(data.stateBuffer));
			context.SetConstant(valueCount);

			context.Dispatch(groupCount, 1, 1);
		});
	}
}
