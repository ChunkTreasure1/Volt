#include "vtpch.h"
#include "PrefixSumTechnique.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphBlackboard.h"

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
			data.stateBuffer = builder.CreateBuffer({ groupCount * sizeof(State), RHI::BufferUsage::StorageBuffer });

			builder.ReadResource(inputBuffer);
			builder.WriteResource(outputBuffer);
			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[pipeline = m_pipeline, groupCount, inputBuffer, outputBuffer](const PrefixSumData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::BufferView> inputBufferView = resources.GetBuffer(inputBuffer)->GetView();
			Ref<RHI::BufferView> outputBufferView = resources.GetBuffer(outputBuffer)->GetView();

			Ref<RHI::StorageBuffer> stateBuffer = resources.GetBuffer(data.stateBuffer);
		
			context.ClearBuffer(stateBuffer, 0);
			
			context.BindPipeline(pipeline);
			context.SetBufferView(inputBufferView, 0, 0);
			context.SetBufferView(outputBufferView, 0, 1);
			context.SetBufferView(stateBuffer->GetView(), 0, 2);

			context.Dispatch(groupCount, 1, 1);
		});
	}
}
