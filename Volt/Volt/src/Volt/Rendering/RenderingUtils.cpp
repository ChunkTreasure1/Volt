#include "vtpch.h"
#include "RenderingUtils.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/RenderGraph/RenderContextUtils.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>

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
			context.SetConstant("indirectArgs"_sh, resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer"_sh, resources.GetBuffer(countBuffer));
			context.SetConstant("threadGroupSize"_sh, groupSize);

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

		auto& data = renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgsWrapped");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs"_sh, resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer"_sh, resources.GetBuffer(countBuffer));
			context.SetConstant("groupSize"_sh, groupSize);

			context.Dispatch(1, 1, 1);
		});

		return data.argsBufferHandle;
	}

	void CopyImage(RenderGraph& renderGraph, RenderGraphResourceHandle sourceImage, RenderGraphResourceHandle destinationImage, const glm::uvec2& renderSize)
	{
		renderGraph.AddPass("Copy Image",
		[&](RenderGraph::Builder& builder) 
		{
			builder.ReadResource(sourceImage);
			builder.WriteResource(destinationImage);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(renderSize.x, renderSize.y, { destinationImage });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("CopyImage");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);
		
			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				context.SetConstant("color"_sh, resources.GetImage2D(sourceImage));
			});

			context.EndRendering();
		});
	}
}
