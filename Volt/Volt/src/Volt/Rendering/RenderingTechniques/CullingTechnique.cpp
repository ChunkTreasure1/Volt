#include "vtpch.h"
#include "CullingTechnique.h"
#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"

#include "Volt/Rendering/RendererCommon.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

namespace Volt
{
	CullingTechnique::CullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	DrawCullingData CullingTechnique::Execute(uint32_t drawCommandCount, uint32_t meshletCount)
	{
		if (drawCommandCount == 0)
		{
			return {};
		}

		m_renderGraph.BeginMarker("Draw Call Culling", { 1.f, 0.f, 0.f, 1.f });

		DrawCullingData data = AddDrawCallCullingPass(drawCommandCount, meshletCount);
		AddTaskSubmitSetupPass(data);

		m_renderGraph.EndMarker();

		return data;
	}

	DrawCullingData CullingTechnique::AddDrawCallCullingPass(uint32_t drawCommandCount, uint32_t meshletCount)
	{
		const auto& uniformBuffers = m_blackboard.Get<UniformBuffersData>();
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		const auto countCmdBufferDesc = RGUtils::CreateBufferDesc<uint32_t>(4, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, "Count And Command Buffer");
		RenderGraphResourceHandle countCmdBufferHandle = m_renderGraph.CreateBuffer(countCmdBufferDesc);

		m_renderGraph.AddClearBufferPass(countCmdBufferHandle, 0, "Clear Count Buffer");

		DrawCullingData& data = m_renderGraph.AddPass<DrawCullingData>("Draw Call Culling Pass",
		[&](RenderGraph::Builder& builder, DrawCullingData& data) 
		{
			data.countCommandBuffer = countCmdBufferHandle;

			{
				const auto desc = RGUtils::CreateBufferDescGPU<MeshTaskCommand>(Math::DivideRoundUp(meshletCount, 32u), "Mesh Task Commands");
				data.taskCommandsBuffer = builder.CreateBuffer(desc);
			}

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.WriteResource(countCmdBufferHandle);

			builder.SetIsComputePass();
		},
		[=](const DrawCullingData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("DrawCallCull");

			context.BindPipeline(pipeline);
			context.SetConstant("countBuffer"_sh, resources.GetBuffer(data.countCommandBuffer));
			context.SetConstant("taskCommands"_sh, resources.GetBuffer(data.taskCommandsBuffer));
			context.SetConstant("viewData"_sh, resources.GetBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("drawCallCount"_sh, drawCommandCount);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);
		
			constexpr uint32_t workGroupSize = 64;

			context.Dispatch(Math::DivideRoundUp(drawCommandCount, workGroupSize), 1, 1);
		});

		return data;
	}

	void CullingTechnique::AddTaskSubmitSetupPass(const DrawCullingData& data)
	{
		m_renderGraph.AddPass("Task Submit Setup Pass",
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(data.countCommandBuffer);
			builder.WriteResource(data.taskCommandsBuffer);
			builder.SetIsComputePass();

			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("TaskSubmitSetup");

			context.BindPipeline(pipeline);
			context.SetConstant("countCommandBuffer"_sh, resources.GetBuffer(data.countCommandBuffer));
			context.SetConstant("taskCommands"_sh, resources.GetBuffer(data.taskCommandsBuffer));
			context.Dispatch(1, 1, 1);
		});
	}
}
