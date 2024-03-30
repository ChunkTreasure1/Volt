#include "sbpch.h"
#include "WindowLayer.h"

#include <Volt/Core/Window.h>
#include <Volt/Core/Application.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/RenderingNew/RenderGraph/RenderGraph.h>
#include <Volt/RenderingNew/RenderGraph/RenderGraphUtils.h>
#include <Volt/RenderingNew/RenderGraph/RenderContextUtils.h>
#include <Volt/RenderingNew/Shader/ShaderMap.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Graphics/Swapchain.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

using namespace Volt;

void WindowLayer::OnAttach()
{
	Volt::WindowProperties windowProperties{};
	windowProperties.width = 512;
	windowProperties.height = 512;
	windowProperties.vsync = true;
	windowProperties.title = "Test Window";
	windowProperties.windowMode = Volt::WindowMode::Windowed;
	windowProperties.iconPath = "";
	windowProperties.cursorPath = "";

	Volt::WindowInheritanceInfo inheritanceInfo{};
	inheritanceInfo.graphicsContext = Volt::Application::Get().GetWindow().GetGraphicsContext();

	m_window = Volt::Window::Create(windowProperties, inheritanceInfo);
	m_window->SetEventCallback(VT_BIND_EVENT_FN(WindowLayer::LayerOnEvent));

	m_commandBuffer = Volt::RHI::CommandBuffer::Create(m_window->GetSwapchainPtr());
}

void WindowLayer::OnDetach()
{
	m_commandBuffer = nullptr;
	m_window = nullptr;
}

void WindowLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppBeginFrameEvent>(VT_BIND_EVENT_FN(WindowLayer::OnBeginFrame));
	dispatcher.Dispatch<Volt::AppPresentFrameEvent>(VT_BIND_EVENT_FN(WindowLayer::OnPresentFrame));
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(WindowLayer::OnRender));
}

void WindowLayer::LayerOnEvent(Volt::Event& e)
{
}

bool WindowLayer::OnBeginFrame(Volt::AppBeginFrameEvent& e)
{
	m_window->BeginFrame();
	return false;
}

bool WindowLayer::OnPresentFrame(Volt::AppPresentFrameEvent& e)
{
	m_window->Present();
	return false;
}

bool WindowLayer::OnRender(Volt::AppRenderEvent& e)
{
	Volt::RenderGraph renderGraph{ m_commandBuffer };

	struct UICommand
	{
		uint32_t type;
		uint32_t primitiveGroup;
		float rotation;
		float scale;
		glm::vec2 radiusHalfSize;
		glm::vec2 pixelPos;
	};

	struct TestUIData
	{
		RenderGraphResourceHandle outputTextureHandle;
		RenderGraphResourceHandle uiCommandsBufferHandle;
	};

	const uint32_t swapchainWidth = m_window->GetSwapchain().GetWidth();
	const uint32_t swapchainHeight = m_window->GetSwapchain().GetHeight();

	TestUIData outData = renderGraph.AddPass<TestUIData>("Test UI",
	[&](RenderGraph::Builder& builder, TestUIData& data)
	{
		{
			data.outputTextureHandle = builder.AddExternalImage2D(m_window->GetSwapchain().GetCurrentImage(), false);
		}

		{
			auto desc = RGUtils::CreateBufferDescGPU<UICommand>(1, "UI Commands");
			data.uiCommandsBufferHandle = builder.CreateBuffer(desc);
		}

		builder.WriteResource(data.outputTextureHandle);
		builder.SetHasSideEffect();
	},
	[=](const TestUIData& data, RenderContext& context, const RenderGraphPassResources& resources)
	{
		std::array<UICommand, 2> cmds;
		{
			UICommand& tcmd = cmds[0];
			tcmd.pixelPos = { 256, 256 };
			tcmd.rotation = 0.f;
			tcmd.scale = 1.f;
			tcmd.radiusHalfSize.x = 100.f;
			tcmd.radiusHalfSize.y = 100.f;
			tcmd.type = 0;
			tcmd.primitiveGroup = 0;
		}

		{
			UICommand& tcmd = cmds[1];
			tcmd.pixelPos = { 256, 400 };
			tcmd.rotation = 0.25f;
			tcmd.scale = 1.f;
			tcmd.radiusHalfSize.x = 100.f;
			tcmd.radiusHalfSize.y = 100.f;
			tcmd.type = 1;
			tcmd.primitiveGroup = 0;
		}

		context.UploadBufferData(data.uiCommandsBufferHandle, cmds.data(), sizeof(UICommand) * cmds.size());

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = ShaderMap::Get("SDFUI");
		auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

		RenderingInfo info = context.CreateRenderingInfo(swapchainWidth, swapchainHeight, { data.outputTextureHandle });

		context.BeginRendering(info);

		RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
		{
			context.SetConstant("commands", resources.GetBuffer(data.uiCommandsBufferHandle));
			context.SetConstant("commandCount", static_cast<uint32_t>(cmds.size()));
			context.SetConstant("renderSize", glm::uvec2{ swapchainWidth, swapchainHeight });
		});

		context.EndRendering();
	});

	{
		Volt::RenderGraphBarrierInfo barrier{};
		barrier.dstStage = Volt::RHI::BarrierStage::RenderTarget;
		barrier.dstAccess = Volt::RHI::BarrierAccess::RenderTarget;
		barrier.dstLayout = Volt::RHI::ImageLayout::Present;

		renderGraph.AddResourceBarrier(outData.outputTextureHandle, barrier);
	}

	renderGraph.Compile();
	renderGraph.Execute();

	return false;
}
