#include "vtpch.h"
#include "SceneRendererNew.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/RenderingNew/RendererCommon.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/ConstantBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>

namespace Volt
{
	SceneRendererNew::SceneRendererNew(const SceneRendererSpecification& specification)
		: m_scene(specification.scene)
	{
		m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics);

		m_shader = RHI::Shader::Create("SimpleTriangle",
		{
			ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/IndirectNew/MeshIndirect_vs.hlsl",
			ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/IndirectNew/MeshIndirect_ps.hlsl"
		}, true);

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = m_shader;
		m_renderPipeline = RHI::RenderPipeline::Create(pipelineInfo);

		// Clear indirect counts
		{
			m_clearIndirectCountsShader = RHI::Shader::Create("ClearIndirectCounts",
			{
				ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/IndirectNew/ClearCountBuffer_cs.hlsl",
			}, true);

			m_clearIndirectCountsPipeline = RHI::ComputePipeline::Create(m_clearIndirectCountsShader);
		}

		// Indirect setup
		{
			m_indirectSetupShader = RHI::Shader::Create("IndirectSetup",
			{
				ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/HLSL/IndirectNew/IndirectSetup_cs.hlsl",
			}, true);

			m_indirectSetupPipeline = RHI::ComputePipeline::Create(m_indirectSetupShader);
		}

		// Create render target
		{
			RHI::ImageSpecification spec{};
			spec.width = specification.initialResolution.x;
			spec.height = specification.initialResolution.y;
			spec.usage = RHI::ImageUsage::Attachment;
			spec.generateMips = false;

			m_outputImage = RHI::Image2D::Create(spec);
		}

		// Constant buffer
		{
			m_constantBuffer = RHI::ConstantBuffer::Create(sizeof(glm::mat4) * 2, nullptr);
		}

		// Storage buffers
		{
			m_indirectCommandsBuffer = RHI::StorageBuffer::Create(1, sizeof(IndirectBatchNew), RHI::MemoryUsage::Indirect | RHI::MemoryUsage::CPUToGPU);
			m_indirectCountsBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t), RHI::MemoryUsage::Indirect);
			m_drawToObjectIDBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t));
			m_transformsBuffer = RHI::StorageBuffer::Create(1, sizeof(glm::mat4), RHI::MemoryUsage::CPUToGPU);
		}

		// Descriptor table
		{
			RHI::DescriptorTableSpecification descriptorTableSpec{};
			descriptorTableSpec.shader = m_shader;
			m_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
			m_descriptorTable->SetBufferView(m_constantBuffer->GetView(), 0, 0);
			m_descriptorTable->SetBufferView(m_transformsBuffer->GetView(), 0, 1);
			m_descriptorTable->SetBufferView(m_drawToObjectIDBuffer->GetView(), 0, 2);
		}

		// Indirect setup descriptor table
		{
			RHI::DescriptorTableSpecification spec{};
			spec.shader = m_indirectSetupShader;
			m_indirectSetupDescriptorTable = RHI::DescriptorTable::Create(spec);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCommandsBuffer->GetView(), 0, 1);
			m_indirectSetupDescriptorTable->SetBufferView(m_drawToObjectIDBuffer->GetView(), 0, 2);
		}

		// Indirect setup descriptor table
		{
			RHI::DescriptorTableSpecification spec{};
			spec.shader = m_clearIndirectCountsShader;
			m_indirectCountDescriptorTable = RHI::DescriptorTable::Create(spec);
			m_indirectCountDescriptorTable->SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
		}
	}

	SceneRendererNew::~SceneRendererNew()
	{
		m_commandBuffer = nullptr;
	}

	void SceneRendererNew::OnRenderEditor(Ref<Camera> camera)
	{
		OnRender(camera);
	}

	void SceneRendererNew::Resize(const uint32_t width, const uint32_t height)
	{
		m_width = width;
		m_height = height;
		m_shouldResize = true;
	}

	Ref<RHI::Image2D> SceneRendererNew::GetFinalImage()
	{
		return m_outputImage;
	}

	void SceneRendererNew::OnRender(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		if (m_scene.lock()->GetRenderScene()->IsInvalid())
		{
			Invalidate();
		}

		if (m_shouldResize)
		{
			m_outputImage->Invalidate(m_width, m_height);
			m_shouldResize = false;
		}

		// Update camera
		{
			glm::mat4* mats = m_constantBuffer->Map<glm::mat4>();
			mats[0] = camera->GetProjection();
			mats[1] = camera->GetView();

			m_constantBuffer->Unmap();
		}

		m_commandBuffer->Begin();

		// Entry barriers
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.oldState = RHI::ResourceState::IndirectArgument;
			barrier.newState = RHI::ResourceState::UnorderedAccess;
			barrier.resource = m_indirectCountsBuffer;

			RHI::ResourceBarrierInfo barrier1{};
			barrier1.oldState = RHI::ResourceState::IndirectArgument;
			barrier1.newState = RHI::ResourceState::UnorderedAccess;
			barrier1.resource = m_indirectCommandsBuffer;

			m_commandBuffer->ResourceBarrier({ barrier, barrier1 });
		}

		// Clear counts buffer
		{
			m_commandBuffer->BindPipeline(m_clearIndirectCountsPipeline);
			m_commandBuffer->BindDescriptorTable(m_indirectCountDescriptorTable);
			RHI::ShaderDataBuffer pushConstantBuffer = m_clearIndirectCountsShader->GetConstantsBuffer();
			pushConstantBuffer.SetMemberData("size", static_cast<uint32_t>(m_activeRenderObjects.size()));

			m_commandBuffer->PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()), 0);

			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_activeRenderObjects.size() / 256) + 1u);
			m_commandBuffer->Dispatch(dispatchCount, 1, 1);
		}

		// Middle barrier
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.oldState = RHI::ResourceState::UnorderedAccess;
			barrier.newState = RHI::ResourceState::UnorderedAccess;
			barrier.resource = m_indirectCountsBuffer;

			RHI::ResourceBarrierInfo barrier1{};
			barrier1.oldState = RHI::ResourceState::NonPixelShaderRead;
			barrier1.newState = RHI::ResourceState::UnorderedAccess;
			barrier1.resource = m_drawToObjectIDBuffer;

			m_commandBuffer->ResourceBarrier({ barrier, barrier1 });
		}

		// Setup indirect
		{
			m_commandBuffer->BindPipeline(m_indirectSetupPipeline);
			m_commandBuffer->BindDescriptorTable(m_indirectSetupDescriptorTable);

			RHI::ShaderDataBuffer pushConstantBuffer = m_indirectSetupShader->GetConstantsBuffer();
			pushConstantBuffer.SetMemberData("drawCallCount", static_cast<uint32_t>(m_activeRenderObjects.size()));

			m_commandBuffer->PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()), 0);

			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_activeRenderObjects.size() / 256) + 1u);
			m_commandBuffer->Dispatch(dispatchCount, 1, 1);
		}

		// Final barrier
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.oldState = RHI::ResourceState::UnorderedAccess;
			barrier.newState = RHI::ResourceState::IndirectArgument;
			barrier.resource = m_indirectCountsBuffer;

			RHI::ResourceBarrierInfo barrier1{};
			barrier1.oldState = RHI::ResourceState::UnorderedAccess;
			barrier1.newState = RHI::ResourceState::NonPixelShaderRead;
			barrier1.resource = m_drawToObjectIDBuffer;

			RHI::ResourceBarrierInfo barrier2{};
			barrier2.oldState = RHI::ResourceState::UnorderedAccess;
			barrier2.newState = RHI::ResourceState::IndirectArgument;
			barrier2.resource = m_indirectCommandsBuffer;

			m_commandBuffer->ResourceBarrier({ barrier, barrier1, barrier2 });
		}

		RHI::Rect2D scissor = { 0, 0, m_width, m_height };
		RHI::Viewport viewport{};
		viewport.width = static_cast<float>(m_width);
		viewport.height = static_cast<float>(m_height);
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
			barrier.resource = m_outputImage;

			m_commandBuffer->ResourceBarrier({ barrier });
		}

		RHI::AttachmentInfo attInfo{};
		attInfo.view = m_outputImage->GetView();
		attInfo.clearColor = { 0.1f, 0.1f, 0.1f, 1.f };
		attInfo.clearMode = RHI::ClearMode::Clear;

		RHI::RenderingInfo renderingInfo{};
		renderingInfo.colorAttachments = { attInfo };
		renderingInfo.renderArea = scissor;

		m_commandBuffer->BeginRendering(renderingInfo);
		m_commandBuffer->BindPipeline(m_renderPipeline);
		m_commandBuffer->BindDescriptorTable(m_descriptorTable);

		m_commandBuffer->DrawIndirectCount(m_indirectCommandsBuffer, 0, m_indirectCountsBuffer, 0, static_cast<uint32_t>(m_activeRenderObjects.size()), sizeof(IndirectBatchNew));

		m_commandBuffer->EndRendering();

		// Shader read barrier
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.oldState = RHI::ResourceState::RenderTarget;
			barrier.newState = RHI::ResourceState::PixelShaderRead;
			barrier.resource = m_outputImage;

			m_commandBuffer->ResourceBarrier({ barrier });
		}

		m_commandBuffer->End();
		m_commandBuffer->Execute();
	}

	void SceneRendererNew::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		m_activeRenderObjects = m_scene.lock()->GetRenderScene()->GetObjects();

		if (m_transformsBuffer->GetSize() < m_activeRenderObjects.size())
		{
			m_transformsBuffer->Resize(static_cast<uint32_t>(m_activeRenderObjects.size()));
			m_descriptorTable->SetBufferView(m_transformsBuffer->GetView(), 0, 1);
		}

		if (m_indirectCommandsBuffer->GetSize() < m_activeRenderObjects.size())
		{
			m_indirectCommandsBuffer->Resize(static_cast<uint32_t>(m_activeRenderObjects.size()));
			m_indirectCountsBuffer->Resize(static_cast<uint32_t>(m_activeRenderObjects.size()));
			m_drawToObjectIDBuffer->Resize(static_cast<uint32_t>(m_activeRenderObjects.size()));

			m_indirectSetupDescriptorTable->SetBufferView(m_drawToObjectIDBuffer->GetView(), 0, 2);

			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCommandsBuffer->GetView(), 0, 1);
			m_indirectSetupDescriptorTable->SetBufferView(m_drawToObjectIDBuffer->GetView(), 0, 2);

			m_indirectCountDescriptorTable->SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
		
			m_descriptorTable->SetBufferView(m_drawToObjectIDBuffer->GetView(), 0, 2);
		}

		std::sort(std::execution::par, m_activeRenderObjects.begin(), m_activeRenderObjects.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.mesh < rhs.mesh)
			{
				return true;
			}

			if (lhs.mesh > rhs.mesh)
			{
				return false;
			}

			if (lhs.subMeshIndex < rhs.subMeshIndex)
			{
				return true;
			}

			if (lhs.subMeshIndex > rhs.subMeshIndex)
			{
				return false;
			}

			return false;
		});

		std::unordered_map<Mesh*, uint32_t> indexVertexBufferMap;
		uint32_t currentIndexVertexBufferIndex = 0;

		glm::mat4* transforms = m_transformsBuffer->Map<glm::mat4>();
		auto indirectCommands = m_indirectCommandsBuffer->Map<IndirectBatchNew>();

		for (uint32_t index = 0; auto & obj : m_activeRenderObjects)
		{
			auto meshPtr = obj.mesh.get();

			if (indexVertexBufferMap.contains(meshPtr))
			{
				obj.vertexBufferIndex = indexVertexBufferMap.at(meshPtr);
			}
			else
			{
				const uint32_t vertexBufferIndex = currentIndexVertexBufferIndex++;
				obj.vertexBufferIndex = vertexBufferIndex;

				indexVertexBufferMap[meshPtr] = vertexBufferIndex;
				m_descriptorTable->SetBufferView(obj.mesh->GetVertexPositionsBuffer()->GetView(), 1, 0, vertexBufferIndex);
				m_descriptorTable->SetBufferView(obj.mesh->GetIndexStorageBuffer()->GetView(), 4, 0, vertexBufferIndex);
			}

			Entity entity{ obj.entity, m_scene.lock().get() };
			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);

			indirectCommands[index].command.vertexCount = subMesh.indexCount;
			indirectCommands[index].command.instanceCount = 1;
			indirectCommands[index].command.firstVertex = subMesh.indexStartOffset;
			indirectCommands[index].command.firstInstance = 0;
			indirectCommands[index].objectId = index;
			indirectCommands[index].batchId = 0;
			indirectCommands[index].vertexBufferIndex = obj.vertexBufferIndex;

			transforms[index] = entity.GetTransform();

			index++;
		}

		m_indirectCommandsBuffer->Unmap();
		m_transformsBuffer->Unmap();

		m_scene.lock()->GetRenderScene()->SetValid();
	}
}
