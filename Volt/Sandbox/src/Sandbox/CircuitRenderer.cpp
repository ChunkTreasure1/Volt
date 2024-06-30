#include "sbpch.h"
#include "CircuitRenderer.h"

#include "CircuitRendererStructs.h"

#include <Volt/Rendering/RenderGraph/RenderGraph.h>
#include <Volt/Rendering/RenderGraph/RenderGraphUtils.h>
#include <Volt/Rendering/RenderGraph/RenderGraphBlackboard.h>
#include <Volt/Rendering/RenderGraph/RenderGraphExecutionThread.h>
#include <Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h>
#include <Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h>
#include <Volt/Rendering/RenderGraph/RenderContextUtils.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <Volt/Core/Profiling.h>

#include <Circuit/Window/CircuitWindow.h>
#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

CircuitRenderer::CircuitRenderer(Circuit::CircuitWindow& window)
	: m_targetCircuitWindow(window),
	m_targetWindow(Volt::Application::GetWindowManager().GetWindow(m_targetCircuitWindow.GetInterfaceWindowHandle()))
{
	m_width = 0;
	m_height = 0;
}

void CircuitRenderer::OnRender()
{
	VT_PROFILE_FUNCTION();

	if (m_targetCircuitWindow.GetWindowSize().x < 0 ||
		m_targetCircuitWindow.GetWindowSize().y < 0)
	{
		VT_CORE_ERROR("Window size must be non-zero");
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

	Volt::RenderGraph renderGraph{ m_commandBuffer };
	Volt::RenderGraphBlackboard rgBlackboard{};
	CircuitOutputData& outData = rgBlackboard.Add<CircuitOutputData>();

	AddCircuitPrimitivesPass(renderGraph, rgBlackboard);

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

void CircuitRenderer::AddCircuitPrimitivesPass(Volt::RenderGraph& renderGraph, Volt::RenderGraphBlackboard& blackboard)
{
	const uint32_t swapchainWidth = m_targetWindow.GetSwapchain().GetWidth();
	const uint32_t swapchainHeight = m_targetWindow.GetSwapchain().GetHeight();

	blackboard.Get<CircuitOutputData>() = renderGraph.AddPass<CircuitOutputData>("Test UI",
	[&](Volt::RenderGraph::Builder& builder, CircuitOutputData& data)
	{
		{
			data.outputTextureHandle = builder.AddExternalImage2D(m_targetWindow.GetSwapchain().GetCurrentImage());
		}

		{
			auto desc = Volt::RGUtils::CreateBufferDescGPU<Circuit::CircuitDrawCommand>(1, "UI Commands");
			data.uiCommandsBufferHandle = builder.CreateBuffer(desc);
		}

		builder.WriteResource(data.outputTextureHandle);
		builder.SetHasSideEffect();
	},
	[=](const CircuitOutputData& data, Volt::RenderContext& context, const Volt::RenderGraphPassResources& resources)
	{
		std::array<Circuit::CircuitDrawCommand, 2> cmds;
		{
			Circuit::CircuitDrawCommand& tcmd = cmds[0];
			tcmd.pixelPos = { 256, 256 };
			tcmd.rotation = 0.f;
			tcmd.scale = 1.f;
			tcmd.radiusHalfSize.x = 100.f;
			tcmd.radiusHalfSize.y = 100.f;
			tcmd.type = 0;
			tcmd.primitiveGroup = 0;
		}

		{
			Circuit::CircuitDrawCommand& tcmd = cmds[1];
			tcmd.pixelPos = { 256, 400 };
			tcmd.rotation = 0.25f;
			tcmd.scale = 1.f;
			tcmd.radiusHalfSize.x = 100.f;
			tcmd.radiusHalfSize.y = 100.f;
			tcmd.type = 1;
			tcmd.primitiveGroup = 0;
		}

		context.MappedBufferUpload(data.uiCommandsBufferHandle, cmds.data(), sizeof(Circuit::CircuitDrawCommand) * cmds.size());

		Volt::RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = Volt::ShaderMap::Get("SDFUI");
		auto pipeline = Volt::ShaderMap::GetRenderPipeline(pipelineInfo);

		Volt::RenderingInfo info = context.CreateRenderingInfo(swapchainWidth, swapchainHeight, { data.outputTextureHandle });

		context.BeginRendering(info);

		Volt::RCUtils::DrawFullscreenTriangle(context, pipeline, [&](Volt::RenderContext& context)
		{
			context.SetConstant("commands"_sh, resources.GetBuffer(data.uiCommandsBufferHandle));
			context.SetConstant("commandCount"_sh, static_cast<uint32_t>(cmds.size()));
			context.SetConstant("renderSize"_sh, glm::uvec2{ swapchainWidth, swapchainHeight });
		});

		context.EndRendering();
	});
}
