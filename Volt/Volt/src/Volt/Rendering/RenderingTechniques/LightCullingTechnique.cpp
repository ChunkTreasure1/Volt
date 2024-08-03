#include "vtpch.h"
#include "LightCullingTechnique.h"

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

namespace Volt
{
	LightCullingTechnique::LightCullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	LightCullingData LightCullingTechnique::Execute()
	{
		const auto& uniformBuffers = m_blackboard.Get<UniformBuffersData>();
		const auto& lightBuffers = m_blackboard.Get<LightBuffersData>();
		const auto& preDepthData = m_blackboard.Get<PreDepthData>();
		const auto& renderData = m_blackboard.Get<RenderData>();

		constexpr uint32_t MAX_LIGHT_COUNT_PER_TILE = 512;

		const uint32_t tileCountX = Math::DivideRoundUp(renderData.renderSize.x, TILE_SIZE);
		const uint32_t tileCountY = Math::DivideRoundUp(renderData.renderSize.y, TILE_SIZE);

		LightCullingData& data = m_renderGraph.AddPass<LightCullingData>("Light Culling",
		[&](RenderGraph::Builder& builder, LightCullingData& data) 
		{
			{
				const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(tileCountX * tileCountY * MAX_LIGHT_COUNT_PER_TILE, "Visible Point Lights");
				data.visiblePointLightsBuffer = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(tileCountX * tileCountY * MAX_LIGHT_COUNT_PER_TILE, "Visible Spot Lights");
				data.visibleSpotLightsBuffer = builder.CreateBuffer(desc);
			}
		
			builder.ReadResource(preDepthData.depth);
			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(lightBuffers.pointLightsBuffer);
			builder.ReadResource(lightBuffers.spotLightsBuffer);

			builder.SetIsComputePass();
		},
		[=](const LightCullingData& data, RenderContext& context) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("LightTileBinning");

			context.BindPipeline(pipeline);
			context.SetConstant("depthTexture"_sh, preDepthData.depth);
			context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);
			context.SetConstant("pointLights"_sh, lightBuffers.pointLightsBuffer);
			context.SetConstant("spotLights"_sh, lightBuffers.spotLightsBuffer);
			context.SetConstant("visiblePointLightIndices"_sh, data.visiblePointLightsBuffer);
			context.SetConstant("visibleSpotLightIndices"_sh, data.visibleSpotLightsBuffer);
			context.SetConstant("tileCount"_sh, glm::uvec2{ tileCountX, tileCountY });
		
			context.Dispatch(tileCountX, tileCountY, 1u);
		});

		return data;
	}
}
