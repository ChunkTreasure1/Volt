#include "circuitpch.h"

#include "Rendering/CircuitRenderer.h"
#include "Rendering/CircuitRendererStructs.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraphExecutionThread.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/Resources/BindlessResourcesManager.h>

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Graphics/DeviceQueue.h>
#include <RHIModule/Images/SamplerState.h>

#include <CoreUtilities/Profiling/Profiling.h>

#include <Circuit/Window/CircuitWindow.h>
#include <Circuit/CircuitManager.h>

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <LogModule/Log.h>

#include <RenderCore/Shader/ShaderMap.h>

using namespace Volt;

namespace Circuit
{
	Circuit::CircuitRenderer::CircuitRenderer(CircuitWindow& targetCircuitWindow)
		: m_targetCircuitWindow(targetCircuitWindow), m_targetWindow(Volt::WindowManager::Get().GetWindow(targetCircuitWindow.GetWindowHandle())),
		m_commandBufferSet(m_targetWindow.GetSwapchain().GetFramesInFlight())
	{
		m_width = 0;
		m_height = 0;

		RHI::SamplerStateCreateInfo samplerInfo{};
		samplerInfo.minFilter = RHI::TextureFilter::Linear;
		samplerInfo.magFilter = RHI::TextureFilter::Linear;
		samplerInfo.mipFilter = RHI::TextureFilter::Linear;
		samplerInfo.wrapMode = RHI::TextureWrap::Clamp;

		m_linearSampler = RHI::SamplerState::Create(samplerInfo);
		m_linearSamplerResourceHandle = BindlessResourcesManager::Get().RegisterSamplerState(m_linearSampler);
	}

	CircuitRenderer::~CircuitRenderer()
	{
		BindlessResourcesManager::Get().UnregisterSamplerState(m_linearSamplerResourceHandle);
	}

	void CircuitRenderer::OnRender()
	{
		VT_PROFILE_FUNCTION();

		if (m_targetCircuitWindow.GetWindowSize().x < 0 ||
			m_targetCircuitWindow.GetWindowSize().y < 0)
		{
			VT_LOG(Error, "Window size must be non-zero");
			return;
		}

		const uint32_t swapchainWidth = m_targetWindow.GetSwapchain().GetWidth();
		const uint32_t swapchainHeight = m_targetWindow.GetSwapchain().GetHeight();

		if (m_width != swapchainWidth ||
			m_height != swapchainHeight)
		{
			m_width = swapchainWidth;
			m_height = swapchainHeight;
		}

		Volt::RenderGraph renderGraph{ m_commandBufferSet.IncrementAndGetCommandBuffer() };
		Volt::RenderGraphBlackboard rgBlackboard{};

		CircuitOutputData& outData = AddCircuitPrimitivesPass(renderGraph, rgBlackboard);

		{
			Volt::RenderGraphBarrierInfo barrier{};
			barrier.dstStage = Volt::RHI::BarrierStage::RenderTarget;
			barrier.dstAccess = Volt::RHI::BarrierAccess::RenderTarget;
			barrier.dstLayout = Volt::RHI::ImageLayout::Present;

			renderGraph.AddResourceBarrier(outData.outputTextureHandle, barrier);
		}

		renderGraph.Compile();
		renderGraph.Execute();
	}

	CircuitOutputData& CircuitRenderer::AddCircuitPrimitivesPass(Volt::RenderGraph& renderGraph, Volt::RenderGraphBlackboard& blackboard)
	{
		const uint32_t swapchainWidth = m_targetWindow.GetSwapchain().GetWidth();
		const uint32_t swapchainHeight = m_targetWindow.GetSwapchain().GetHeight();

		Volt::RenderGraphBufferHandle uiCommandsBufferHandle;
		std::vector<Circuit::CircuitDrawCommand> cmds = m_targetCircuitWindow.GetDrawCommands();

		const size_t commandsCount = cmds.size();
		if (commandsCount > 0)
		{
			auto desc = Volt::RGUtils::CreateBufferDescGPU<Circuit::CircuitDrawCommand>(cmds.size(), "UI Commands");
			uiCommandsBufferHandle = renderGraph.CreateBuffer(desc);
			renderGraph.AddStagedBufferUpload(uiCommandsBufferHandle, cmds.data(), sizeof(Circuit::CircuitDrawCommand) * cmds.size(), "UI Commands");
		}

		CircuitOutputData& outData = renderGraph.AddPass<CircuitOutputData>("Test UI",
		[&](Volt::RenderGraph::Builder& builder, CircuitOutputData& data)
		{
			{
				data.outputTextureHandle = builder.AddExternalImage(m_targetWindow.GetSwapchain().GetCurrentImage());
			}

			data.uiCommandsBufferHandle = uiCommandsBufferHandle;


			builder.WriteResource(data.outputTextureHandle);
			builder.ReadResource(data.uiCommandsBufferHandle);
			builder.SetHasSideEffect();
		},
		[=](const CircuitOutputData& data, Volt::RenderContext& context)
		{
			Volt::RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = Volt::ShaderMap::Get("SDFUI");
			auto pipeline = Volt::ShaderMap::GetRenderPipeline(pipelineInfo);

			Volt::RenderingInfo info = context.CreateRenderingInfo(swapchainWidth, swapchainHeight, { data.outputTextureHandle });

			context.BeginRendering(info);

			Volt::RCUtils::DrawFullscreenTriangle(context, pipeline, [&](Volt::RenderContext& context)
			{
				context.SetConstant("commands"_sh, data.uiCommandsBufferHandle);
				context.SetConstant("linearSampler"_sh, m_linearSamplerResourceHandle);
				context.SetConstant("commandCount"_sh, static_cast<uint32_t>(commandsCount));
				context.SetConstant("renderSize"_sh, glm::uvec2{ swapchainWidth, swapchainHeight });
			});

			context.EndRendering();
		});

		return outData;
	}
}
