#include "vtpch.h"
#include "SceneRendererNew.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/RenderingNew/RendererCommon.h"
#include "Volt/RenderingNew/RendererNew.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Math/Math.h"

#include "Volt/Components/LightComponents.h"
#include "Volt/Components/Components.h"

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Buffers/UniformBufferSet.h>
#include <VoltRHI/Buffers/StorageBufferSet.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>

inline static constexpr uint32_t CAMERA_BUFFER_BINDING = 0;
inline static constexpr uint32_t DIRECTIONAL_LIGHT_BINDING = 1;

namespace Volt
{
	namespace Utility
	{
		const size_t HashMeshSubMesh(Ref<Mesh> mesh, const SubMesh& subMesh)
		{
			size_t result = std::hash<void*>()(mesh.get());
			result = Math::HashCombine(result, subMesh.GetHash());

			return result;
		}
	}

	SceneRendererNew::SceneRendererNew(const SceneRendererSpecification& specification)
		: m_scene(specification.scene)
	{
		m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics);

		m_shader = RHI::Shader::Create("SimpleTriangle",
		{
			ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/Indirect/MeshIndirect_vs.hlsl",
			ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/Indirect/MeshIndirect_ps.hlsl"
		}, true);

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = m_shader;
		m_renderPipeline = RHI::RenderPipeline::Create(pipelineInfo);

		// Clear indirect counts
		{
			m_clearIndirectCountsShader = RHI::Shader::Create("ClearIndirectCounts",
			{
				ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/Indirect/ClearCountBuffer_cs.hlsl",
			}, true);

			m_clearIndirectCountsPipeline = RHI::ComputePipeline::Create(m_clearIndirectCountsShader);
		}

		// Indirect setup
		{
			m_indirectSetupShader = RHI::Shader::Create("IndirectSetup",
			{
				ProjectManager::GetEngineDirectory() / "Engine/Shaders/Source/Indirect/IndirectSetup_cs.hlsl",
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

			spec.format = RHI::Format::D32_SFLOAT;
			m_depthImage = RHI::Image2D::Create(spec);
		}

		// Constant buffer
		{
			m_constantBufferSet = RHI::UniformBufferSet::Create(RendererNew::GetFramesInFlight());
			m_constantBufferSet->Add<CameraDataNew>(5, CAMERA_BUFFER_BINDING);
			m_constantBufferSet->Add<DirectionalLightData>(5, DIRECTIONAL_LIGHT_BINDING);
		}

		// Storage buffers
		{
			m_indirectCommandsBuffer = RHI::StorageBuffer::Create(1, sizeof(IndirectGPUCommandNew), RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::CPUToGPU);
			m_indirectCountsBuffer = RHI::StorageBuffer::Create(2, sizeof(uint32_t), RHI::BufferUsage::IndirectBuffer);

			m_drawToInstanceOffsetBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t));
			m_instanceOffsetToObjectIDBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t));
			m_indirectDrawDataBuffer = RHI::StorageBuffer::Create(1, sizeof(IndirectDrawData), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU);
		}

		// Sampler state
		{
			RHI::SamplerStateCreateInfo info{};
			info.minFilter = RHI::TextureFilter::Linear;
			info.magFilter = RHI::TextureFilter::Linear;
			info.mipFilter = RHI::TextureFilter::Linear;
			info.wrapMode = RHI::TextureWrap::Repeat;

			m_samplerState = RHI::SamplerState::Create(info);
		}

		// Descriptor table
		{
			RHI::DescriptorTableSpecification descriptorTableSpec{};
			descriptorTableSpec.shader = m_shader;
			m_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
			m_descriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 0);
			m_descriptorTable->SetBufferView(m_indirectDrawDataBuffer->GetView(), 0, 1);
			m_descriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 2);

			m_descriptorTable->SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, CAMERA_BUFFER_BINDING), 5, CAMERA_BUFFER_BINDING);
			m_descriptorTable->SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, DIRECTIONAL_LIGHT_BINDING), 5, DIRECTIONAL_LIGHT_BINDING);

			m_descriptorTable->SetSamplerState(m_samplerState, 5, 3);
		}

		// Indirect setup descriptor table
		{
			RHI::DescriptorTableSpecification spec{};
			spec.shader = m_indirectSetupShader;
			m_indirectSetupDescriptorTable = RHI::DescriptorTable::Create(spec);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCommandsBuffer->GetView(), 0, 1);
			m_indirectSetupDescriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 2);
			m_indirectSetupDescriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 3);
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
			m_depthImage->Invalidate(m_width, m_height);
			m_shouldResize = false;
		}

		RenderGraph renderGraph{ m_commandBuffer };
		
		renderGraph.AddPass("Test Pass",
		[](RenderGraph::Builder& builder)
		{
			//auto buffer = builder.CreateBuffer({ 32 });
		},
		[](const RenderGraphPassResources& resources)
		{
		});

		UpdateBuffers(camera);

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
			pushConstantBuffer.SetMemberData("size", static_cast<uint32_t>(m_currentActiveCommandCount));

			m_commandBuffer->PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()), 0);

			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_currentActiveCommandCount / 256) + 1u);
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
			barrier1.resource = m_drawToInstanceOffsetBuffer;

			RHI::ResourceBarrierInfo barrier2{};
			barrier2.oldState = RHI::ResourceState::NonPixelShaderRead;
			barrier2.newState = RHI::ResourceState::UnorderedAccess;
			barrier2.resource = m_instanceOffsetToObjectIDBuffer;

			m_commandBuffer->ResourceBarrier({ barrier, barrier1, barrier2 });
		}

		// Setup indirect
		{
			m_commandBuffer->BindPipeline(m_indirectSetupPipeline);
			m_commandBuffer->BindDescriptorTable(m_indirectSetupDescriptorTable);

			RHI::ShaderDataBuffer pushConstantBuffer = m_indirectSetupShader->GetConstantsBuffer();
			pushConstantBuffer.SetMemberData("drawCallCount", static_cast<uint32_t>(m_currentActiveCommandCount));

			m_commandBuffer->PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()), 0);

			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_currentActiveCommandCount / 256) + 1u);
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
			barrier1.resource = m_drawToInstanceOffsetBuffer;

			RHI::ResourceBarrierInfo barrier2{};
			barrier2.oldState = RHI::ResourceState::UnorderedAccess;
			barrier2.newState = RHI::ResourceState::NonPixelShaderRead;
			barrier2.resource = m_instanceOffsetToObjectIDBuffer;

			RHI::ResourceBarrierInfo barrier3{};
			barrier3.oldState = RHI::ResourceState::UnorderedAccess;
			barrier3.newState = RHI::ResourceState::IndirectArgument;
			barrier3.resource = m_indirectCommandsBuffer;

			m_commandBuffer->ResourceBarrier({ barrier, barrier1, barrier2, barrier3 });
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

		RHI::AttachmentInfo depthInfo{};
		depthInfo.view = m_depthImage->GetView();
		depthInfo.clearColor = { 0.f, 1.f, 0.f, 0.f };
		depthInfo.clearMode = RHI::ClearMode::Clear;

		RHI::RenderingInfo renderingInfo{};
		renderingInfo.colorAttachments = { attInfo };
		renderingInfo.renderArea = scissor;
		renderingInfo.depthAttachmentInfo = depthInfo;

		m_commandBuffer->BeginRendering(renderingInfo);
		m_commandBuffer->BindPipeline(m_renderPipeline);
		m_commandBuffer->BindDescriptorTable(m_descriptorTable);

		m_commandBuffer->DrawIndirectCount(m_indirectCommandsBuffer, 0, m_indirectCountsBuffer, 0, m_currentActiveCommandCount, sizeof(IndirectGPUCommandNew));

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

		auto scenePtr = m_scene.lock();
		auto renderScene = scenePtr->GetRenderScene();

		renderScene->PrepareForUpdate();
		const uint32_t individualMeshCount = renderScene->GetIndividualMeshCount();
		const uint32_t renderObjectCount = renderScene->GetRenderObjectCount();

		if (m_indirectCommandsBuffer->GetSize() < individualMeshCount)
		{
			m_indirectCommandsBuffer->Resize(individualMeshCount);
			m_indirectSetupDescriptorTable->SetBufferView(m_indirectCommandsBuffer->GetView(), 0, 1);
		}

		if (m_indirectDrawDataBuffer->GetSize() < renderObjectCount)
		{
			m_indirectDrawDataBuffer->Resize(renderObjectCount);
			m_descriptorTable->SetBufferView(m_indirectDrawDataBuffer->GetView(), 0, 1);
		}

		if (m_instanceOffsetToObjectIDBuffer->GetSize() < renderObjectCount)
		{
			m_instanceOffsetToObjectIDBuffer->Resize(renderObjectCount);
			m_drawToInstanceOffsetBuffer->Resize(renderObjectCount);

			m_indirectSetupDescriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 2);
			m_indirectSetupDescriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 3);

			m_descriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 0);
			m_descriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 2);
		}

		UpdateDescriptorTable(*renderScene);

		uint32_t currentMeshIndex = 0;

		std::unordered_map<Mesh*, uint32_t> meshDataMap;
		std::unordered_map<size_t, uint32_t> meshSubMeshIndexMap;

		IndirectDrawData* drawDataBuffer = m_indirectDrawDataBuffer->Map<IndirectDrawData>();
		IndirectGPUCommandNew* indirectCommands = m_indirectCommandsBuffer->Map<IndirectGPUCommandNew>();

		m_currentActiveCommandCount = 0;
		for (uint32_t index = 0; const auto & obj : *renderScene)
		{
			auto meshPtr = obj.mesh.get();
			uint32_t meshIndex = 0;

			if (meshDataMap.contains(meshPtr))
			{
				meshIndex = meshDataMap.at(meshPtr);
			}
			else
			{
				meshIndex = currentMeshIndex++;
				meshDataMap[meshPtr] = meshIndex;

				m_descriptorTable->SetBufferView(obj.mesh->GetVertexPositionsBuffer()->GetView(), 1, 0, meshIndex);
				m_descriptorTable->SetBufferView(obj.mesh->GetVertexMaterialBuffer()->GetView(), 2, 0, meshIndex);
				m_descriptorTable->SetBufferView(obj.mesh->GetVertexAnimationBuffer()->GetView(), 3, 0, meshIndex);
				m_descriptorTable->SetBufferView(obj.mesh->GetIndexStorageBuffer()->GetView(), 4, 0, meshIndex);
			}

			Entity entity{ obj.entity, m_scene.lock().get() };

			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);

			const size_t meshSubMeshHash = Utility::HashMeshSubMesh(obj.mesh, subMesh);
			if (meshSubMeshIndexMap.contains(meshSubMeshHash))
			{
				const uint32_t cmdIndex = meshSubMeshIndexMap.at(meshSubMeshHash);
				indirectCommands[cmdIndex].command.instanceCount++;
			}
			else
			{
				meshSubMeshIndexMap[meshSubMeshHash] = m_currentActiveCommandCount;

				indirectCommands[m_currentActiveCommandCount].command.vertexCount = subMesh.indexCount;
				indirectCommands[m_currentActiveCommandCount].command.instanceCount = 1;
				indirectCommands[m_currentActiveCommandCount].command.firstVertex = subMesh.indexStartOffset;
				indirectCommands[m_currentActiveCommandCount].command.firstInstance = 0;
				indirectCommands[m_currentActiveCommandCount].objectId = index;
				indirectCommands[m_currentActiveCommandCount].meshId = meshIndex;
				m_currentActiveCommandCount++;
			}

			drawDataBuffer[index].meshId = meshIndex;
			drawDataBuffer[index].vertexStartOffset = subMesh.vertexStartOffset;
			drawDataBuffer[index].transform = entity.GetTransform();

			index++;
		}

		m_indirectDrawDataBuffer->Unmap();
		m_indirectCommandsBuffer->Unmap();

		m_scene.lock()->GetRenderScene()->SetValid();
	}

	void SceneRendererNew::UpdateBuffers(Ref<Camera> camera)
	{
		UpdateCameraBuffer(camera);
		UpdateLightBuffers();
	}

	void SceneRendererNew::UpdateCameraBuffer(Ref<Camera> camera)
	{
		if (!camera)
		{
			return;
		}

		auto cameraBuffer = m_constantBufferSet->Get(5, CAMERA_BUFFER_BINDING, m_commandBuffer->GetCurrentIndex());

		CameraDataNew* cameraData = cameraBuffer->Map<CameraDataNew>();

		cameraData->projection = camera->GetProjection();
		cameraData->view = camera->GetView();
		cameraData->inverseView = glm::inverse(camera->GetView());
		cameraData->inverseProjection = glm::inverse(camera->GetProjection());
		cameraData->position = glm::vec4(camera->GetPosition(), 1.f);

		cameraBuffer->Unmap();
	}

	void SceneRendererNew::UpdateLightBuffers()
	{
		auto scenePtr = m_scene.lock();
		auto& registry = scenePtr->GetRegistry();

		// Directional Light
		{
			DirectionalLightData dirLightData{};

			registry.ForEach<DirectionalLightComponent, TransformComponent>([&](Wire::EntityId id, const DirectionalLightComponent& lightComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				Entity entity = { id, scenePtr.get() };

				const glm::vec3 dir = glm::rotate(entity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;
				dirLightData.direction = glm::vec4{ dir.x, dir.y, dir.z, 0.f };
				dirLightData.color = lightComp.color;
				dirLightData.intensity = lightComp.intensity;
			});

			m_constantBufferSet->Get(5, DIRECTIONAL_LIGHT_BINDING, m_commandBuffer->GetCurrentIndex())->SetData<DirectionalLightData>(dirLightData);
		}
	}

	void SceneRendererNew::UpdateDescriptorTable(RenderScene& renderScene)
	{
		for (uint32_t index = 0; const auto & mesh : renderScene.GetIndividualMeshes())
		{
			m_descriptorTable->SetBufferView(mesh.lock()->GetVertexPositionsBuffer()->GetView(), 1, 0, index);
			m_descriptorTable->SetBufferView(mesh.lock()->GetVertexMaterialBuffer()->GetView(), 2, 0, index);
			m_descriptorTable->SetBufferView(mesh.lock()->GetVertexAnimationBuffer()->GetView(), 3, 0, index);
			m_descriptorTable->SetBufferView(mesh.lock()->GetIndexStorageBuffer()->GetView(), 4, 0, index);
			index++;
		}
	}
}
