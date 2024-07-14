#include "vtpch.h"
#include "CullingTechnique.h"

#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"

#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/RenderingUtils.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Rendering/Camera/Camera.h"

namespace Volt
{
	CullingTechnique::CullingTechnique(RenderGraph& rg, RenderGraphBlackboard& blackboard)
		: m_renderGraph(rg), m_blackboard(blackboard)
	{
	}

	CullPrimitivesData CullingTechnique::Execute(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode, const glm::vec2& renderSize, const uint32_t instanceCount)
	{
		m_renderGraph.BeginMarker("Culling", { 0.f, 1.f, 0.f, 1.f });

		auto objectsData = AddCullObjectsPass(camera, renderScene, cullingMode);
		auto meshletsData = AddCullMeshletsPass(camera, renderScene, cullingMode, objectsData);
		auto primitivesData = AddCullPrimitivesPass(camera, renderScene, meshletsData, renderSize, instanceCount);

		m_renderGraph.EndMarker();

		return primitivesData;
	}

	CullObjectsData CullingTechnique::AddCullObjectsPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode)
	{
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		RenderGraphResourceHandle meshletCountBuffer = 0;
		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Meshlet Count");
			meshletCountBuffer = m_renderGraph.CreateBuffer(desc);
			m_renderGraph.AddClearBufferPass(meshletCountBuffer, 0, "Clear Meshlet Count");
		}

		CullObjectsData& data = m_renderGraph.AddPass<CullObjectsData>("Cull Objects",
		[&](RenderGraph::Builder& builder, CullObjectsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<glm::uvec2>(std::max(renderScene->GetMeshletCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Meshlet To Object ID And Offset");
				data.meshletToObjectIdAndOffset = builder.CreateBuffer(desc);
			}

			data.meshletCount = meshletCountBuffer;

			builder.WriteResource(data.meshletCount);
			
			GPUSceneData::SetupInputs(builder, gpuSceneData);
			
			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](const CullObjectsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			const uint32_t commandCount = renderScene->GetRenderObjectCount();
			const uint32_t dispatchCount = Math::DivideRoundUp(commandCount, 256u);

			auto pipeline = ShaderMap::GetComputePipeline("CullObjects");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("meshletCount"_sh, resources.GetBuffer(data.meshletCount));
			context.SetConstant("meshletToObjectIdAndOffset"_sh, resources.GetBuffer(data.meshletToObjectIdAndOffset));
			context.SetConstant("objectCount"_sh, commandCount);
			context.SetConstant("cullingMode"_sh, static_cast<uint32_t>(cullingMode));

			const auto projection = camera->GetProjection();
			const auto frustumAABB = camera->GetOrthographicFrustum();

			const glm::mat4 projTranspose = glm::transpose(projection);

			const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
			const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

			context.SetConstant("frustum0"_sh, frustumX.x);
			context.SetConstant("frustum1"_sh, frustumX.z);
			context.SetConstant("frustum2"_sh, frustumY.y);
			context.SetConstant("frustum3"_sh, frustumY.z);

			context.SetConstant("orthographicFrustumMin"_sh, frustumAABB.GetMin());
			context.SetConstant("orthographicFrustumMax"_sh, frustumAABB.GetMax());

			context.SetConstant("nearPlane"_sh, camera->GetNearPlane());
			context.SetConstant("farPlane"_sh, camera->GetFarPlane());
			context.SetConstant("viewMatrix"_sh, camera->GetView());

			context.Dispatch(dispatchCount, 1, 1);
		});

		return data;
	}

	CullMeshletsData CullingTechnique::AddCullMeshletsPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode, const CullObjectsData& cullObjectsData)
	{
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		RenderGraphResourceHandle argsBufferHandle = RenderingUtils::GenerateIndirectArgs(m_renderGraph, cullObjectsData.meshletCount, 256, "Cull Meshlets Indirect Args");

		RenderGraphResourceHandle survivingMeshletCountBuffer = 0;
		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Surviving Meshlets Count");
			survivingMeshletCountBuffer = m_renderGraph.CreateBuffer(desc);
			m_renderGraph.AddClearBufferPass(survivingMeshletCountBuffer, 0, "Clear Surviving Meshlet Count");
		}

		CullMeshletsData& cullMeshletsData = m_renderGraph.AddPass<CullMeshletsData>("Cull Meshlets",
		[&](RenderGraph::Builder& builder, CullMeshletsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetMeshletCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Surviving Meshlets");
				data.survivingMeshlets = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Surviving Meshlets Count");
				data.survivingMeshletCount = builder.CreateBuffer(desc);
			}

			data.survivingMeshletCount = survivingMeshletCountBuffer;

			builder.WriteResource(survivingMeshletCountBuffer);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.ReadResource(cullObjectsData.meshletCount);
			builder.ReadResource(cullObjectsData.meshletToObjectIdAndOffset);
			builder.ReadResource(argsBufferHandle, RenderGraphResourceState::IndirectArgument);

			builder.SetIsComputePass();
		},
		[=](const CullMeshletsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("CullMeshlets");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("survivingMeshlets"_sh, resources.GetBuffer(data.survivingMeshlets));
			context.SetConstant("survivingMeshletCount"_sh, resources.GetBuffer(data.survivingMeshletCount));
			context.SetConstant("meshletCount"_sh, resources.GetBuffer(cullObjectsData.meshletCount));
			context.SetConstant("meshletToObjectIdAndOffset"_sh, resources.GetBuffer(cullObjectsData.meshletToObjectIdAndOffset));
			context.SetConstant("cullingMode"_sh, static_cast<uint32_t>(cullingMode));

			const auto projection = camera->GetProjection();
			const auto frustumAABB = camera->GetOrthographicFrustum();

			const glm::mat4 projTranspose = glm::transpose(projection);

			const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
			const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

			context.SetConstant("frustum0"_sh, frustumX.x);
			context.SetConstant("frustum1"_sh, frustumX.z);
			context.SetConstant("frustum2"_sh, frustumY.y);
			context.SetConstant("frustum3"_sh, frustumY.z);

			context.SetConstant("orthographicFrustumMin"_sh, frustumAABB.GetMin());
			context.SetConstant("orthographicFrustumMax"_sh, frustumAABB.GetMax());

			context.SetConstant("nearPlane"_sh, camera->GetNearPlane());
			context.SetConstant("farPlane"_sh, camera->GetFarPlane());
			context.SetConstant("viewMatrix"_sh, camera->GetView());

			context.DispatchIndirect(argsBufferHandle, 0);
		});

		return cullMeshletsData;
	}

	CullPrimitivesData CullingTechnique::AddCullPrimitivesPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullMeshletsData& cullMeshletsData, const glm::vec2& renderSize, const uint32_t instanceCount)
	{
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		RenderGraphResourceHandle argsBufferHandle = RenderingUtils::GenerateIndirectArgsWrapped(m_renderGraph, cullMeshletsData.survivingMeshletCount, 1, "Cull Primitives Indirect Args");

		RenderGraphResourceHandle drawCommandBuffer = 0;

		{
			const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectDrawIndexedCommand>(1, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, "Meshlet Draw Command");
			drawCommandBuffer = m_renderGraph.CreateBuffer(desc);
		
			RHI::IndirectDrawIndexedCommand command{};
			command.indexCount = 0;
			command.firstInstance = 0;
			command.firstIndex = 0;
			command.vertexOffset = 0;
			command.instanceCount = instanceCount;

			m_renderGraph.AddStagedBufferUpload(drawCommandBuffer, &command, sizeof(RHI::IndirectDrawIndexedCommand), "Upload indirect command");
		}

		CullPrimitivesData& primitivesData = m_renderGraph.AddPass<CullPrimitivesData>("Cull Primitives",
		[&](RenderGraph::Builder& builder, CullPrimitivesData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndexCount(), 1u), RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndexBuffer, RHI::MemoryUsage::GPU, "Meshlet Index Buffer");
				data.indexBuffer = builder.CreateBuffer(desc);
			}

			data.drawCommand = drawCommandBuffer;

			builder.WriteResource(data.drawCommand);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.ReadResource(cullMeshletsData.survivingMeshlets);
			builder.ReadResource(cullMeshletsData.survivingMeshletCount);
			builder.ReadResource(argsBufferHandle, RenderGraphResourceState::IndirectArgument);

			builder.SetIsComputePass();
		},
		[=](const CullPrimitivesData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("CullPrimitives");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("indexBuffer"_sh, resources.GetBuffer(data.indexBuffer));
			context.SetConstant("drawCommand"_sh, resources.GetBuffer(data.drawCommand));
			context.SetConstant("survivingMeshlets"_sh, resources.GetBuffer(cullMeshletsData.survivingMeshlets));
			context.SetConstant("survivingMeshletCount"_sh, resources.GetBuffer(cullMeshletsData.survivingMeshletCount));
			context.SetConstant("viewMatrix"_sh, camera->GetView());
			context.SetConstant("projectionMatrix"_sh, camera->GetProjection());
			context.SetConstant("renderSize"_sh, renderSize);

			context.DispatchIndirect(argsBufferHandle, 0);
		});

		return primitivesData;
	}
}
