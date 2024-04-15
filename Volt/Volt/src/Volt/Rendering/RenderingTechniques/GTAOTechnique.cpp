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

	void GTAOTechnique::AddGTAOPasses(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera, const glm::uvec2& renderSize)
	{
		renderGraph.BeginMarker("GTAO", { 0.f, 1.f, 0.f, 1.f });

		AddPrefilterDepthPass(renderGraph, blackboard, camera, renderSize);
		AddMainPass(renderGraph, blackboard);
		AddDenoisePass(renderGraph, blackboard);

		renderGraph.EndMarker();
	}

	void GTAOTechnique::AddPrefilterDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera, const glm::uvec2& renderSize)
	{
		constexpr uint32_t GTAO_PREFILTERED_DEPTH_MIP_COUNT = 5;

		m_constants.ViewportSize = renderSize;
		m_constants.ViewportPixelSize = { 1.f / static_cast<float>(renderSize.x), 1.f / static_cast<float>(renderSize.y) };

		const auto& projectionMatrix = camera->GetProjection();
		
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
			desc.width = renderSize.x;
			desc.height = renderSize.y;
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
			context.SetConstant("outDepthMIP0", resources.GetImage2D(data.prefilteredDepth, 0));
			context.SetConstant("outDepthMIP1", resources.GetImage2D(data.prefilteredDepth, 1));
			context.SetConstant("outDepthMIP2", resources.GetImage2D(data.prefilteredDepth, 2));
			context.SetConstant("outDepthMIP3", resources.GetImage2D(data.prefilteredDepth, 3));
			context.SetConstant("outDepthMIP4", resources.GetImage2D(data.prefilteredDepth, 4));
			context.SetConstant("sourceDepth", resources.GetImage2D(preDepthData.depth));
			context.SetConstant("pointClampSampler", pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize", data.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize", data.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts", data.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV", data.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul", data.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd", data.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize", data.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius", data.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange", data.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier", data.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0", data.constants.Padding0);
			context.SetConstant("constants.FinalValuePower", data.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta", data.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower", data.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation", data.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset", data.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex", data.constants.NoiseIndex);


			const uint32_t dispatchX = Math::DivideRoundUp(renderSize.x, 16u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderSize.y, 16u);
		
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
			context.SetConstant("aoTerm", resources.GetImage2D(data.aoOutput));
			context.SetConstant("edges", resources.GetImage2D(data.edgesOutput));
			context.SetConstant("srcDepth", resources.GetImage2D(prefilterDepthData.prefilteredDepth));
			context.SetConstant("viewspaceNormals", resources.GetImage2D(preDepthData.normals));
			context.SetConstant("pointClampSampler", pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize", prefilterDepthData.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize", prefilterDepthData.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts", prefilterDepthData.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV", prefilterDepthData.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul", prefilterDepthData.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd", prefilterDepthData.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize", prefilterDepthData.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius", prefilterDepthData.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange", prefilterDepthData.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier", prefilterDepthData.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0", prefilterDepthData.constants.Padding0);
			context.SetConstant("constants.FinalValuePower", prefilterDepthData.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta", prefilterDepthData.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower", prefilterDepthData.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation", prefilterDepthData.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset", prefilterDepthData.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex", prefilterDepthData.constants.NoiseIndex);

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
			context.SetConstant("finalAOTerm", resources.GetImage2D(data.outputImage));
			context.SetConstant("aoTerm", resources.GetImage2D(gtaoData.aoOutput));
			context.SetConstant("edges", resources.GetImage2D(gtaoData.edgesOutput));
			context.SetConstant("pointClampSampler", pointClampSampler->GetResourceHandle());
			context.SetConstant("constants.ViewportSize", prefilterDepthData.constants.ViewportSize);
			context.SetConstant("constants.ViewportPixelSize", prefilterDepthData.constants.ViewportPixelSize);
			context.SetConstant("constants.DepthUnpackConsts", prefilterDepthData.constants.DepthUnpackConsts);
			context.SetConstant("constants.CameraTanHalfFOV", prefilterDepthData.constants.CameraTanHalfFOV);
			context.SetConstant("constants.NDCToViewMul", prefilterDepthData.constants.NDCToViewMul);
			context.SetConstant("constants.NDCToViewAdd", prefilterDepthData.constants.NDCToViewAdd);
			context.SetConstant("constants.NDCToViewMul_x_PixelSize", prefilterDepthData.constants.NDCToViewMul_x_PixelSize);
			context.SetConstant("constants.EffectRadius", prefilterDepthData.constants.EffectRadius);
			context.SetConstant("constants.EffectFalloffRange", prefilterDepthData.constants.EffectFalloffRange);
			context.SetConstant("constants.RadiusMultiplier", prefilterDepthData.constants.RadiusMultiplier);
			context.SetConstant("constants.Padding0", prefilterDepthData.constants.Padding0);
			context.SetConstant("constants.FinalValuePower", prefilterDepthData.constants.FinalValuePower);
			context.SetConstant("constants.DenoiseBlurBeta", prefilterDepthData.constants.DenoiseBlurBeta);
			context.SetConstant("constants.SampleDistributionPower", prefilterDepthData.constants.SampleDistributionPower);
			context.SetConstant("constants.ThinOccluderCompensation", prefilterDepthData.constants.ThinOccluderCompensation);
			context.SetConstant("constants.DepthMIPSamplingOffset", prefilterDepthData.constants.DepthMIPSamplingOffset);
			context.SetConstant("constants.NoiseIndex", prefilterDepthData.constants.NoiseIndex);
		
			const uint32_t dispatchX = Math::DivideRoundUp(renderSize.x, 8u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderSize.y, 8u);

			context.Dispatch(dispatchX, dispatchY, 1);
		});
	}
}

