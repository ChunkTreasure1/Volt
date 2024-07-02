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
#include <Circuit/CircuitManager.h>
#include <Circuit/Window/WindowInterfaceDefines.h>

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

CircuitRenderer::CircuitRenderer(Volt::WindowHandle& windowHandle)
	: m_targetCircuitWindow(*Circuit::CircuitManager::Get().GetWindows().at(static_cast<InterfaceWindowHandle>(windowHandle))),
	m_targetWindow(Volt::Application::GetWindowManager().GetWindow(m_targetCircuitWindow.GetInterfaceWindowHandle()))
{
	m_width = 0;
	m_height = 0;

	m_commandBuffer = Volt::RHI::CommandBuffer::Create(m_targetWindow.GetSwapchainPtr());
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

	Volt::RenderGraphResourceHandle uiCommandsBufferHandle;
	std::vector<Circuit::CircuitDrawCommand> cmds = m_targetCircuitWindow.GetDrawCommands();
	/*cmds.resize(2);
	{
		Circuit::CircuitDrawCommand& tcmd = cmds[0];
		tcmd.pixelPos = { 256, 256 };
		tcmd.rotation = 0.f;
		tcmd.scale = 1.f;
		tcmd.radiusHalfSize.x = 100.f;
		tcmd.radiusHalfSize.y = 100.f;
		tcmd.type = Circuit::CircuitPrimitiveType::Rect;
		tcmd.primitiveGroup = 0;
	}

	{
		Circuit::CircuitDrawCommand& tcmd = cmds[1];
		tcmd.pixelPos = { 256, 400 };
		tcmd.rotation = 0.25f;
		tcmd.scale = 1.f;
		tcmd.radiusHalfSize.x = 100.f;
		tcmd.radiusHalfSize.y = 100.f;
		tcmd.type = Circuit::CircuitPrimitiveType::Circle;
		tcmd.primitiveGroup = 0;
	}*/

	const size_t commandsCount = cmds.size();
	if (commandsCount > 0)
	{
		auto desc = Volt::RGUtils::CreateBufferDescGPU<Circuit::CircuitDrawCommand>(1, "UI Commands");
		uiCommandsBufferHandle = renderGraph.CreateBuffer(desc);
		renderGraph.AddStagedBufferUpload(uiCommandsBufferHandle, cmds.data(), sizeof(Circuit::CircuitDrawCommand) * cmds.size(), "UI Commands");
	}

	CircuitOutputData& outData = renderGraph.AddPass<CircuitOutputData>("Test UI",
	[&](Volt::RenderGraph::Builder& builder, CircuitOutputData& data)
	{
		{
			data.outputTextureHandle = builder.AddExternalImage2D(m_targetWindow.GetSwapchain().GetCurrentImage());
		}

		data.uiCommandsBufferHandle = uiCommandsBufferHandle;


		builder.WriteResource(data.outputTextureHandle);
		builder.ReadResource(data.uiCommandsBufferHandle);
		builder.SetHasSideEffect();
	},
	[=](const CircuitOutputData& data, Volt::RenderContext& context, const Volt::RenderGraphPassResources& resources)
	{
		Volt::RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = Volt::ShaderMap::Get("SDFUI");
		auto pipeline = Volt::ShaderMap::GetRenderPipeline(pipelineInfo);

		Volt::RenderingInfo info = context.CreateRenderingInfo(swapchainWidth, swapchainHeight, { data.outputTextureHandle });

		context.BeginRendering(info);

		Volt::RCUtils::DrawFullscreenTriangle(context, pipeline, [&](Volt::RenderContext& context)
		{
			context.SetConstant("commands"_sh, resources.GetBuffer(data.uiCommandsBufferHandle));
			context.SetConstant("commandCount"_sh, static_cast<uint32_t>(commandsCount));
			context.SetConstant("renderSize"_sh, glm::uvec2{ swapchainWidth, swapchainHeight });
		});

		context.EndRendering();
	});

	return outData;
}
