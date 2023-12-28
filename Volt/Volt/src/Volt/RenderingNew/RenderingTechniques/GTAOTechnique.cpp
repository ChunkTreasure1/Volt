#include "vtpch.h"
#include "GTAOTechnique.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphUtils.h"
#include "Volt/RenderingNew/SceneRendererStructs.h"
#include "Volt/RenderingNew/Shader/ShaderMap.h"
#include "Volt/RenderingNew/RendererNew.h"

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
			auto pointClampSampler = RendererNew::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();

			context.BindPipeline(pipeline);
			context.SetConstant(resources.GetImage2D(data.prefilteredDepth, 0));
			context.SetConstant(resources.GetImage2D(data.prefilteredDepth, 1));
			context.SetConstant(resources.GetImage2D(data.prefilteredDepth, 2));
			context.SetConstant(resources.GetImage2D(data.prefilteredDepth, 3));
			context.SetConstant(resources.GetImage2D(data.prefilteredDepth, 4));
			context.SetConstant(resources.GetImage2D(preDepthData.depth));
			context.SetConstant(pointClampSampler->GetResourceHandle());
			context.SetConstant(0); // padding
			context.SetConstant(data.constants);

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
			auto pointClampSampler = RendererNew::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();

			context.BindPipeline(pipeline);
			context.SetConstant(resources.GetImage2D(data.aoOutput));
			context.SetConstant(resources.GetImage2D(data.edgesOutput));
			context.SetConstant(resources.GetImage2D(prefilterDepthData.prefilteredDepth));
			context.SetConstant(resources.GetImage2D(preDepthData.normals));
			context.SetConstant(pointClampSampler->GetResourceHandle());
			context.SetConstant(glm::uvec3{ 0 }); // padding
			context.SetConstant(prefilterDepthData.constants);

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
			builder.SetHasSideEffect();
		},
		[=](const GTAOOutput& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GTAODenoise");
			auto pointClampSampler = RendererNew::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureFilter::Nearest, RHI::TextureWrap::Clamp>();
		
			context.BindPipeline(pipeline);
			context.SetConstant(resources.GetImage2D(data.outputImage));
			context.SetConstant(resources.GetImage2D(gtaoData.aoOutput));
			context.SetConstant(resources.GetImage2D(gtaoData.edgesOutput));
			context.SetConstant(pointClampSampler->GetResourceHandle());
			context.SetConstant(prefilterDepthData.constants);
		
			const uint32_t dispatchX = Math::DivideRoundUp(renderSize.x, 8u);
			const uint32_t dispatchY = Math::DivideRoundUp(renderSize.y, 8u);

			context.Dispatch(dispatchX, dispatchY, 1);
		});
	}
}

