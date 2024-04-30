#include "vtpch.h"
#include "TAATechnique.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/RenderGraph/RenderContextUtils.h"

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>

namespace Volt
{
	TAATechnique::TAATechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	TAAData TAATechnique::Execute(Ref<RHI::Image2D> previousColor, const glm::uvec2& renderSize)
	{
		const auto& shadingData = m_blackboard.Get<ShadingOutputData>();

		TAAData& data = m_renderGraph.AddPass<TAAData>("TAA",
		[&](RenderGraph::Builder& builder, TAAData& data) 
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(renderSize.x, renderSize.y, RHI::ImageUsage::AttachmentStorage, "TAA Output");
				data.taaOutput = builder.CreateImage2D(desc);
			}

			data.previousColor = builder.AddExternalImage2D(previousColor);

			builder.ReadResource(data.previousColor);
			builder.ReadResource(shadingData.colorOutput);

		},
		[=](const TAAData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			RenderingInfo info = context.CreateRenderingInfo(renderSize.x, renderSize.y, { data.taaOutput });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("TAAResolve");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context) 
			{
				context.SetConstant("currentColor"_sh, resources.GetImage2D(shadingData.colorOutput));
				context.SetConstant("previousColor"_sh, resources.GetImage2D(data.previousColor));
			});

			context.EndRendering();
		});

		return data;
	}
}
