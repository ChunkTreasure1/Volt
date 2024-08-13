#include "vtpch.h"
#include "RenderingUtils.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RHIModule/Pipelines/RenderPipeline.h>

namespace Volt::RenderingUtils
{
	RenderGraphResourceHandle GenerateIndirectArgs(RenderGraph& renderGraph, RenderGraphBufferHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphBufferHandle argsBufferHandle
				;
		};

		RenderGraphBufferHandle outHandle;

		renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);
			outHandle = data.argsBufferHandle;

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs"_sh, data.argsBufferHandle);
			context.SetConstant("countBuffer"_sh, countBuffer);
			context.SetConstant("threadGroupSize"_sh, groupSize);

			context.Dispatch(1, 1, 1);
		});

		return outHandle;
	}

	RenderGraphResourceHandle GenerateIndirectArgsWrapped(RenderGraph& renderGraph, RenderGraphBufferHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphBufferHandle argsBufferHandle;
		};

		auto& data = renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgsWrapped");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs"_sh, data.argsBufferHandle);
			context.SetConstant("countBuffer"_sh, countBuffer);
			context.SetConstant("groupSize"_sh, groupSize);

			context.Dispatch(1, 1, 1);
		});

		return data.argsBufferHandle;
	}

	void CopyImage(RenderGraph& renderGraph, RenderGraphImageHandle sourceImage, RenderGraphImageHandle destinationImage, const glm::uvec2& renderSize)
	{
		renderGraph.AddPass("Copy Image",
		[&](RenderGraph::Builder& builder) 
		{
			builder.ReadResource(sourceImage);
			builder.WriteResource(destinationImage);
		},
		[=](RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(renderSize.x, renderSize.y, { destinationImage });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("CopyImage");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);
		
			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				context.SetConstant("color"_sh, sourceImage);
			});

			context.EndRendering();
		});
	}
}
