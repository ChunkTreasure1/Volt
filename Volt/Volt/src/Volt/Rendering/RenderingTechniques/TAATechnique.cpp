#include "vtpch.h"
#include "TAATechnique.h"

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/Shader/ShaderMap.h>

#include <RHIModule/Pipelines/RenderPipeline.h>

namespace Volt
{
	TAATechnique::TAATechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	TAAData TAATechnique::Execute(RefPtr<RHI::Image> previousColor, RenderGraphImageHandle velocityTexture)
	{
		const auto& shadingData = m_blackboard.Get<ShadingOutputData>();
		const auto& renderData = m_blackboard.Get<RenderData>();

		TAAData& data = m_renderGraph.AddPass<TAAData>("TAA",
		[&](RenderGraph::Builder& builder, TAAData& data) 
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(renderData.renderSize.x, renderData.renderSize.y, RHI::ImageUsage::AttachmentStorage, "TAA Output");
				data.taaOutput = builder.CreateImage(desc);
			}

			if (!previousColor)
			{
				data.previousColor = builder.AddExternalImage(Renderer::GetDefaultResources().whiteTexture->GetImage());
			}
			else
			{
				data.previousColor = builder.AddExternalImage(previousColor);
			}

			builder.ReadResource(velocityTexture);
			builder.ReadResource(data.previousColor);
			builder.ReadResource(shadingData.colorOutput);

		},
		[=](const TAAData& data, RenderContext& context) 
		{
			RenderingInfo info = context.CreateRenderingInfo(renderData.renderSize.x, renderData.renderSize.y, { data.taaOutput });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("TAAResolve");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context) 
			{
				context.SetConstant("currentColor"_sh, shadingData.colorOutput);
				context.SetConstant("previousColor"_sh, data.previousColor);
				context.SetConstant("velocityTexture"_sh, velocityTexture);
				context.SetConstant("pointSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest>()->GetResourceHandle());
			});

			context.EndRendering();
		});

		return data;
	}
}
