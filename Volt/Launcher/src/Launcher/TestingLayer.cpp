#include "TestingLayer.h"

#include <VoltRHI/Shader/ShaderCompiler.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageView.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Buffers/VertexBuffer.h>
#include <VoltRHI/Buffers/IndexBuffer.h>
#include <VoltRHI/Buffers/ConstantBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <Volt/Project/ProjectManager.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/UIUtility.h>

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>

using namespace Volt;

//#define SWAPCHAIN_TARGET

void TestingLayer::OnAttach()
{
	m_mesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

#ifdef SWAPCHAIN_TARGET
	m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics, true);
#else
	m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics, false);
#endif

	m_shader = RHI::Shader::Create("SimpleTriangle",
	{
		Volt::ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/MeshIndirect_vs.hlsl",
		Volt::ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/MeshIndirect_ps.hlsl"
	}, true);

	RHI::RenderPipelineCreateInfo pipelineInfo{};
	pipelineInfo.shader = m_shader;

	m_renderPipeline = RHI::RenderPipeline::Create(pipelineInfo);

	// Create render target
	{
		RHI::ImageSpecification spec{};
		spec.width = 512;
		spec.height = 512;
		spec.usage = RHI::ImageUsage::Attachment;
		spec.generateMips = false;

		m_renderTarget = RHI::Image2D::Create(spec);
	}

	// Constant buffer
	{
		const glm::mat4 projection = glm::perspective(glm::radians(60.f), 16.f / 9.f, 1000.f, 0.1f);
		const glm::mat4 view = glm::lookAt(glm::vec3{ 0.f, 0.f, -200.f }, glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 0.f, 1.f, 0.f });

		glm::mat4 arr[2] = { projection, view };

		m_constantBuffer = RHI::ConstantBuffer::Create(sizeof(glm::mat4) * 2, arr);
	}

	// Storage buffers
	{
		m_indirectCommandsBuffer = RHI::StorageBuffer::Create(1, sizeof(RHI::IndirectIndexedCommand), RHI::MemoryUsage::Indirect | RHI::MemoryUsage::CPUToGPU);
		m_transformsBuffer = RHI::StorageBuffer::Create(100 * 100 * 100, sizeof(glm::mat4), RHI::MemoryUsage::CPUToGPU);

		glm::mat4* transforms = m_transformsBuffer->Map<glm::mat4>();
		for (uint32_t x = 0; x < 100; x++)
		{
			for (uint32_t y = 0; y < 100; y++)
			{
				for (uint32_t z = 0; z < 100; z++)
				{
					transforms[x + y + z] = glm::translate(glm::mat4{ 1.f }, { 100.f * x, 100.f * y, 100.f * z });
				}
			}
		}
		m_transformsBuffer->Unmap();


		auto indirectCommandsBuffer = m_indirectCommandsBuffer->Map<RHI::IndirectIndexedCommand>();
		indirectCommandsBuffer[0].indexCount = m_mesh->GetSubMeshes().at(0).indexCount;
		indirectCommandsBuffer[0].instanceCount = 100 * 100 * 100;
		indirectCommandsBuffer[0].firstIndex = 0;
		indirectCommandsBuffer[0].vertexOffset = 0;
		indirectCommandsBuffer[0].firstInstance = 0;

		m_indirectCommandsBuffer->Unmap();
	}

	// Descriptor table
	{
		RHI::DescriptorTableSpecification descriptorTableSpec{};
		descriptorTableSpec.shader = m_shader;
		m_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
		m_descriptorTable->SetBufferView(0, 0, m_constantBuffer->GetView());
		m_descriptorTable->SetBufferView(0, 1, m_transformsBuffer->GetView());
	}
}

void TestingLayer::OnDetach()
{
	m_commandBuffer = nullptr;
	m_constantBuffer = nullptr;
	m_renderTarget = nullptr;
}

void TestingLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(TestingLayer::OnRenderEvent));
	dispatcher.Dispatch<Volt::AppImGuiUpdateEvent>(VT_BIND_EVENT_FN(TestingLayer::OnUpdateImGui));
}

bool TestingLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	VT_PROFILE_FUNCTION();

	m_commandBuffer->Begin();

	RHI::Rect2D scissor = { 0, 0, 512, 512 };
	RHI::Viewport viewport{};
	viewport.width = 512;
	viewport.height = 512;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	m_commandBuffer->SetViewports({ viewport });
	m_commandBuffer->SetScissors({ scissor });

	// Render target barrier
	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.oldState = RHI::ResourceState::PixelShaderRead;
		barrier.newState = RHI::ResourceState::RenderTarget;
		barrier.resource = m_renderTarget;

		m_commandBuffer->ResourceBarrier({ barrier });
	}

	RHI::AttachmentInfo attInfo{};
	attInfo.view = m_renderTarget->GetView();
	attInfo.clearColor = { 0.1f, 0.1f, 0.1f, 1.f };
	attInfo.clearMode = RHI::ClearMode::Clear;

	RHI::RenderingInfo renderingInfo{};
	renderingInfo.colorAttachments = { attInfo };
	renderingInfo.renderArea = scissor;

	m_commandBuffer->BeginRendering(renderingInfo);

	m_commandBuffer->BindPipeline(m_renderPipeline);
	m_commandBuffer->BindIndexBuffer(m_mesh->GetIndexBuffer());
	m_commandBuffer->BindVertexBuffers({ m_mesh->GetVertexBuffer() }, 0);
	m_commandBuffer->BindDescriptorTable(m_descriptorTable);

	m_commandBuffer->DrawIndexedIndirect(m_indirectCommandsBuffer, 0, 1, sizeof(RHI::IndirectIndexedCommand));

	m_commandBuffer->EndRendering();

#ifdef SWAPCHAIN_TARGET
	m_commandBuffer->CopyImageToBackBuffer(m_renderTarget);
#else
	// Shader read barrier
	{
		RHI::ResourceBarrierInfo barrier{};
		barrier.oldState = RHI::ResourceState::RenderTarget;
		barrier.newState = RHI::ResourceState::PixelShaderRead;
		barrier.resource = m_renderTarget;

		m_commandBuffer->ResourceBarrier({ barrier });
	}
#endif

	m_commandBuffer->End();
	m_commandBuffer->Execute();

	return false;
}

bool TestingLayer::OnUpdateImGui(Volt::AppImGuiUpdateEvent& e)
{
	if (ImGui::Begin("Render Target"))
	{
		ImTextureID texId = UI::GetTextureID(m_renderTarget);
		ImGui::Image(texId, ImGui::GetContentRegionAvail());

		ImGui::End();
	}

	if (ImGui::Begin("Performance"))
	{
		ImGui::Text("GPU Time: %f ms", m_commandBuffer->GetExecutionTime(0));

		ImGui::End();
	}

	return false;
}
