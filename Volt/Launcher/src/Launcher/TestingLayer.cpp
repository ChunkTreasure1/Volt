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

#include <VoltRHI/Descriptors/DescriptorTable.h>

#include <Volt/Project/ProjectManager.h>
#include <Volt/Asset/AssetManager.h>

#include <glm/glm.hpp>

#include <functional>

using namespace Volt;

void TestingLayer::OnAttach()
{
	m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics, true);
	m_shader = RHI::Shader::Create("SimpleTriangle",
	{
		Volt::ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/ConstantBufferMesh_vs.hlsl",
		Volt::ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/Testing/ConstantBufferMesh_ps.hlsl"
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

	// Descriptor table
	{
		RHI::DescriptorTableSpecification descriptorTableSpec{};
		descriptorTableSpec.shader = m_shader;
		m_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
		m_descriptorTable->SetBufferView(0, 0, m_constantBuffer->GetView());
	}

	m_mesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");
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
	attInfo.clearColor = { 0.5f, 0.1f, 0.1f, 1.f };
	attInfo.clearMode = RHI::ClearMode::Clear;

	RHI::RenderingInfo renderingInfo{};
	renderingInfo.colorAttachments = { attInfo };
	renderingInfo.renderArea = scissor;

	m_commandBuffer->BeginRendering(renderingInfo);

	m_commandBuffer->BindPipeline(m_renderPipeline);
	m_commandBuffer->BindIndexBuffer(m_mesh->GetIndexBuffer());
	m_commandBuffer->BindVertexBuffers({ m_mesh->GetVertexBuffer() }, 0);
	m_commandBuffer->BindDescriptorTable(m_descriptorTable);

	glm::mat4 transform = glm::mat4{ 1.f };
	auto constantsBuffer = m_shader->GetConstantsBuffer();
	constantsBuffer.SetMemberData("transform", transform);

	m_commandBuffer->PushConstants(constantsBuffer.GetBuffer(), static_cast<uint32_t>(constantsBuffer.GetSize()), 0);
	m_commandBuffer->DrawIndexed(m_mesh->GetSubMeshes().at(0).indexCount, 1, 0, 0, 0);

	m_commandBuffer->EndRendering();

	m_commandBuffer->CopyImageToBackBuffer(m_renderTarget);
	m_commandBuffer->End();
	m_commandBuffer->Execute();

	return false;
}

bool TestingLayer::OnUpdateImGui(Volt::AppImGuiUpdateEvent& e)
{
	return false;
}
