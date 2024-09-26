#include "vtpch.h"
#include "Volt/Rendering/RenderingTechniques/PrefixSumTechnique.h"

#include "Volt/Math/Math.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/Shader/ShaderMap.h>

#include <RHIModule/Buffers/BufferView.h>
#include <RHIModule/Buffers/StorageBuffer.h>

namespace Volt
{
	PrefixSumTechnique::PrefixSumTechnique(RenderGraph& rg)
		: m_renderGraph(rg)
	{
	}

	void PrefixSumTechnique::Execute(RenderGraphBufferHandle inputBuffer, RenderGraphBufferHandle outputBuffer, const uint32_t valueCount)
	{
		constexpr uint32_t TG_SIZE = 512;

		struct PrefixSumData
		{
			RenderGraphBufferHandle stateBuffer;
		};

		struct State
		{
			uint32_t aggregate;
			uint32_t prefix;
			uint32_t state;
		};

		const uint32_t groupCount = Math::DivideRoundUp(valueCount, TG_SIZE);

		auto pipeline = ShaderMap::GetComputePipeline("PrefixSum");

		RenderGraphBufferHandle stateBuffer = RenderGraphNullHandle();
		{
			const auto desc = RGUtils::CreateBufferDesc<State>(std::max(groupCount, 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "State Buffer");
			stateBuffer = m_renderGraph.CreateBuffer(desc);
			RGUtils::ClearBuffer(m_renderGraph, stateBuffer, 0, "Clear State Buffer");
		}

		m_renderGraph.AddPass<PrefixSumData>("Prefix Sum",
		[&](RenderGraph::Builder& builder, PrefixSumData& data)
		{
			data.stateBuffer = stateBuffer;
			builder.WriteResource(stateBuffer);
			builder.ReadResource(inputBuffer);
			builder.WriteResource(outputBuffer);
			builder.SetIsComputePass();
		},
		[pipeline, groupCount, inputBuffer, outputBuffer, valueCount](const PrefixSumData& data, RenderContext& context)
		{
			context.BindPipeline(pipeline);
			context.SetConstant("inputValues"_sh, inputBuffer);
			context.SetConstant("outputValues"_sh, outputBuffer);
			context.SetConstant("stateBuffer"_sh, data.stateBuffer);
			context.SetConstant("valueCount"_sh, valueCount);

			context.Dispatch(groupCount, 1, 1);
		});
	}
}
