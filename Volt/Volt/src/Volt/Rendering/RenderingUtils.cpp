#include "vtpch.h"
#include "RenderingUtils.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

namespace Volt::RenderingUtils
{
	RenderGraphResourceHandle GenerateIndirectArgs(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphResourceHandle argsBufferHandle = 0;
		};

		RenderGraphResourceHandle outHandle = 0;

		renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);
			outHandle = data.argsBufferHandle;

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs", resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer", resources.GetBuffer(countBuffer));
			context.SetConstant("threadGroupSize", groupSize);

			context.Dispatch(1, 1, 1);
		});

		return outHandle;
	}

	RenderGraphResourceHandle GenerateIndirectArgsWrapped(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphResourceHandle argsBufferHandle = 0;
		};

		RenderGraphResourceHandle outHandle = 0;

		renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);
			outHandle = data.argsBufferHandle;

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgsWrapped");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs", resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer", resources.GetBuffer(countBuffer));
			context.SetConstant("groupSize", groupSize);

			context.Dispatch(1, 1, 1);
		});

		return outHandle;
	}
}
