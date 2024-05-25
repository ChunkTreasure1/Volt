#include "vtpch.h"
#include "GTAOTechnique.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Math/Math.h"

namespace Volt
{
	struct PrefilterDepthData
	{
		RenderGraphResourceHandle prefilteredDepth;
		GTAOTechnique::GTAOConstants constants{};
	};

	struct GTAOData
	{
		RenderGraphResourceHandle aoOutput;
		RenderGraphResourceHandle edgesOutput;
	};

	GTAOTechnique::GTAOTechnique(uint64_t frameIndex, const GTAOSettings& settings)
		: m_frameIndex(frameIndex)
	{
		// Setup constants
		{
			GTAOConstants constants{};

			///// Settings /////
			constants.EffectRadius = settings.radius;
			constants.EffectFalloffRange = settings.falloffRange;
			constants.DenoiseBlurBeta = 1.2f; // 1 denoise pass
			constants.RadiusMultiplier = settings.radiusMultiplier;
			constants.SampleDistributionPower = 2.f;
			constants.ThinOccluderCompensation = 0.f;
			constants.FinalValuePower = settings.finalValuePower;
			constants.DepthMIPSamplingOffset = 3.3f;
			constants.NoiseIndex = frameIndex % 64;
			constants.Padding0 = 0;

			m_constants = constants;
		}
	}

	void GTAOTechnique::Execute(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		renderGraph.BeginMarker("GTAO", { 0.f, 1.f, 0.f, 1.f });

		AddPrefilterDepthPass(renderGraph, blackboard);
		AddMainPass(renderGraph, blackboard);
		AddDenoisePass(renderGraph, blackboard);

		renderGraph.EndMarker();
	}

	void GTAOTechnique::AddPrefilterDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		constexpr uint32_t GTAO_PREFILTERED_DEPTH_MIP_COUNT = 5;

		const auto& renderData = blackboard.Get<RenderData>();

		m_constants.ViewportSize = renderData.renderSize;
		m_constants.ViewportPixelSize = { 1.f / static_cast<float>(renderData.renderSize.x), 1.f / static_cast<float>(renderData.renderSize.y) };

		const auto& projectionMatrix = renderData.camera->GetProjection();
		
		float depthLinearizeMul = (-projectionMatrix[3][2]);
		float depthLinearizeAdd = (projectionMatrix[2][2]);
		
		// correct the handedness issue
		if (depthLinearizeMul * depthLinearizeAdd < 0.f)
		{
			depthLinearizeAdd = -depthLinearizeAdd;
		}
		
		m_constants.DepthUnpackConsts = { depthLinearizeMul, depthLinearizeAdd };
		
		const float tanHalfFovY = 1.f / (projectionMatrix[1][1]);
		const float tanHalfFovX = 1.f / (projectionMatrix[0][0]);
		
		m_constants.CameraTanHalfFOV = { tanHalfFovX, tanHalfFovY };
		m_constants.NDCToViewMul = { m_constants.CameraTanHalfFOV.x * 2.f, m_constants.CameraTanHalfFOV.y * -2.f };
		m_constants.NDCToViewAdd = { m_constants.CameraTanHalfFOV.x * -1.f, m_constants.CameraTanHalfFOV.y * 1.f };
		m_constants.NDCToViewMul_x_PixelSize = { m_constants.NDCToViewMul.x * m_constants.ViewportPixelSize.x, m_constants.NDCToViewMul.y * m_constants.ViewportPixelSize.y };

		const auto& preDepthData = blackboard.Get<PreDepthData>();

		blackboard.Add<PrefilterDepthData>() = renderGraph.AddPass<PrefilterDepthData>("GTAO Prefilter Depth Pass",
		[&](RenderGraph::Builder& builder, PrefilterDepthData& data) 
		{
			RenderGraphImageDesc desc{};
			desc.format = RHI::PixelFormat::R32_SFLOAT;
			desc.width = renderData.renderSize.x;
			desc.height = renderData.renderSize.y;
			desc.usage = RHI::ImageUsage::Storage;
			desc.mips = GTAO_PREFILTERED_DEPTH_MIP_COUNT;
			desc.name = "GTAO Prefiltered Depth";

			data.prefilteredDepth = builder.CreateImage2D(desc);
			data.constants = m_constants;

			builder.ReadResource(preDepthData.depth);
			builder.SetIsComputePass();
		},
		[=](const PrefilterDepthData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("GTAODepthPrefilter");
			auto pointClampSampler = Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();

			context.BindPipeline(pipeline);
			context.SetConstant("outDepthMIP0"_sh, resources.GetImage2D(data.prefilteredDepth, 0));
			context.SetConstant("outDepthMIP1"_sh, resources.GetImage2D(data.prefilteredDepth, 1));
			context.SetConstant("outDepthMIP2"_sh, resources.GetImage2D(data.prefilteredDepth, 2));
			context.SetConstant("outDepthMIP3"_sh, resources.GetImage2D(data.prefilteredDepth, 3));
			context.SetConstant("outDepthMIP4"_sh, resources.GetImage2D(data.prefilteredDepth, 4));
			context.SetConstant("sourceDepth"_sh, resources.GetImage2D(preDepthData.depth));
			context.SetConstant("pointClampSampler"_sh, pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize"_sh, data.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize"_sh, data.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts"_sh, data.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV"_sh, data.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul"_sh, data.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd"_sh, data.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize"_sh, data.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius"_sh, data.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange"_sh, data.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier"_sh, data.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0"_sh, data.constants.Padding0);
			context.SetConstant("constants.FinalValuePower"_sh, data.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta"_sh, data.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower"_sh, data.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation"_sh, data.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset"_sh, data.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex"_sh, data.constants.NoiseIndex);


			const uint32_t dispatchX = Math::DivideRoundUp(renderData.renderSize.x, 16u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderData.renderSize.y, 16u);
		
			context.Dispatch(dispatchX, dispatchY, 1);
		});
	}

	void GTAOTechnique::AddMainPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& prefilterDepthData = blackboard.Get<PrefilterDepthData>();
		const auto& preDepthData = blackboard.Get<PreDepthData>();

		const glm::uvec2 renderSize = m_constants.ViewportSize;

		blackboard.Add<GTAOData>() = renderGraph.AddPass<GTAOData>("GTAO Main Pass",
		[&](RenderGraph::Builder& builder, GTAOData& data) 
		{
			// AO Texture
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R32_UINT>(renderSize.x, renderSize.y, RHI::ImageUsage::Storage, "GTAO AO Output");
				data.aoOutput = builder.CreateImage2D(desc);
			}

			// Edges Texture
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R8_UNORM>(renderSize.x, renderSize.y, RHI::ImageUsage::Storage, "GTAO Edges Output");
				data.edgesOutput = builder.CreateImage2D(desc);
			}

			builder.ReadResource(prefilterDepthData.prefilteredDepth);
			builder.ReadResource(preDepthData.normals);
			builder.SetIsComputePass();
		},
		[=](const GTAOData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("GTAOMainPass");
			auto pointClampSampler = Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();

			context.BindPipeline(pipeline);
			context.SetConstant("aoTerm"_sh, resources.GetImage2D(data.aoOutput));
			context.SetConstant("edges"_sh, resources.GetImage2D(data.edgesOutput));
			context.SetConstant("srcDepth"_sh, resources.GetImage2D(prefilterDepthData.prefilteredDepth));
			context.SetConstant("viewspaceNormals"_sh, resources.GetImage2D(preDepthData.normals));
			context.SetConstant("pointClampSampler"_sh, pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize"_sh, prefilterDepthData.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize"_sh, prefilterDepthData.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts"_sh, prefilterDepthData.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV"_sh, prefilterDepthData.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul"_sh, prefilterDepthData.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd"_sh, prefilterDepthData.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize"_sh, prefilterDepthData.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius"_sh, prefilterDepthData.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange"_sh, prefilterDepthData.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier"_sh, prefilterDepthData.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0"_sh, prefilterDepthData.constants.Padding0);
			context.SetConstant("constants.FinalValuePower"_sh, prefilterDepthData.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta"_sh, prefilterDepthData.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower"_sh, prefilterDepthData.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation"_sh, prefilterDepthData.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset"_sh, prefilterDepthData.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex"_sh, prefilterDepthData.constants.NoiseIndex);

			const uint32_t dispatchX = Math::DivideRoundUp(renderSize.x, 16u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderSize.y, 16u);

			context.Dispatch(dispatchX, dispatchY, 1);
		});
	}

	void GTAOTechnique::AddDenoisePass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& gtaoData = blackboard.Get<GTAOData>();
		const auto& prefilterDepthData = blackboard.Get<PrefilterDepthData>();

		const glm::uvec2 renderSize = m_constants.ViewportSize;

		blackboard.Add<GTAOOutput>() = renderGraph.AddPass<GTAOOutput>("GTAO Denoise Pass 0",
		[&](RenderGraph::Builder& builder, GTAOOutput& data) 
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R32_UINT>(renderSize.x, renderSize.y, RHI::ImageUsage::Storage, "GTAO Final Output");
				data.outputImage = builder.CreateImage2D(desc);
			}

			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R32_UINT>(renderSize.x, renderSize.y, RHI::ImageUsage::Storage, "GTAO Temp Image");
				data.tempImage = builder.CreateImage2D(desc);
			}

			builder.ReadResource(gtaoData.aoOutput);
			builder.ReadResource(gtaoData.edgesOutput);

			builder.SetIsComputePass();
		},
		[=](const GTAOOutput& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GTAODenoise");
			auto pointClampSampler = Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();
		
			context.BindPipeline(pipeline);
			context.SetConstant("finalAOTerm"_sh, resources.GetImage2D(data.outputImage));
			context.SetConstant("aoTerm"_sh, resources.GetImage2D(gtaoData.aoOutput));
			context.SetConstant("edges"_sh, resources.GetImage2D(gtaoData.edgesOutput));
			context.SetConstant("pointClampSampler"_sh, pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize"_sh, prefilterDepthData.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize"_sh, prefilterDepthData.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts"_sh, prefilterDepthData.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV"_sh, prefilterDepthData.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul"_sh, prefilterDepthData.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd"_sh, prefilterDepthData.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize"_sh, prefilterDepthData.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius"_sh, prefilterDepthData.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange"_sh, prefilterDepthData.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier"_sh, prefilterDepthData.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0"_sh, prefilterDepthData.constants.Padding0);
			context.SetConstant("constants.FinalValuePower"_sh, prefilterDepthData.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta"_sh, prefilterDepthData.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower"_sh, prefilterDepthData.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation"_sh, prefilterDepthData.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset"_sh, prefilterDepthData.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex"_sh, prefilterDepthData.constants.NoiseIndex);
		
			const uint32_t dispatchX = Math::DivideRoundUp(renderSize.x, 8u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderSize.y, 8u);

			context.Dispatch(dispatchX, dispatchY, 1);
		});
	}
}

