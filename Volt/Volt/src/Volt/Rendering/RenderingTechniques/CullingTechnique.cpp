#include "vtpch.h"
#include "CullingTechnique.h"

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Math/Math.h"

#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>

namespace Volt
{
	CullingTechnique::CullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	DrawCullingData CullingTechnique::Execute(const Info& info)
	{
		if (info.drawCommandCount == 0)
		{
			return {};
		}

		m_renderGraph.BeginMarker("Draw Call Culling", { 1.f, 0.f, 0.f, 1.f });

		DrawCullingData data = AddDrawCallCullingPass(info);
		AddTaskSubmitSetupPass(data);

		m_renderGraph.EndMarker();

		return data;
	}

	DrawCullingData CullingTechnique::AddDrawCallCullingPass(const Info& info)
	{
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		const auto countCmdBufferDesc = RGUtils::CreateBufferDesc<uint32_t>(4, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, "Count And Command Buffer");
		RenderGraphBufferHandle countCmdBufferHandle = m_renderGraph.CreateBuffer(countCmdBufferDesc);

		RGUtils::ClearBuffer(m_renderGraph, countCmdBufferHandle, 0, "Clear Count Buffer");

		DrawCullingData& data = m_renderGraph.AddPass<DrawCullingData>("Draw Call Culling Pass",
		[&](RenderGraph::Builder& builder, DrawCullingData& data) 
		{
			data.countCommandBuffer = countCmdBufferHandle;

			{
				const auto desc = RGUtils::CreateBufferDescGPU<MeshTaskCommand>(Math::DivideRoundUp(info.meshletCount, 32u), "Mesh Task Commands");
				data.taskCommandsBuffer = builder.CreateBuffer(desc);
			}

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.WriteResource(countCmdBufferHandle);

			builder.SetHasSideEffect();
			builder.SetIsComputePass();
		},
		[=](const DrawCullingData& data, RenderContext& context) 
		{
			auto pipeline = ShaderMap::GetComputePipeline("DrawCallCull");

			context.BindPipeline(pipeline);
			context.SetConstant("countBuffer"_sh, data.countCommandBuffer);
			context.SetConstant("taskCommands"_sh, data.taskCommandsBuffer);

			context.SetConstant("viewMatrix"_sh, info.viewMatrix);
			context.SetConstant("cullingFrustum"_sh, info.cullingFrustum);
			context.SetConstant("nearPlane"_sh, info.nearPlane);
			context.SetConstant("farPlane"_sh, info.farPlane);
			context.SetConstant("drawCallCount"_sh, info.drawCommandCount);
			context.SetConstant("cullingType"_sh, static_cast<uint32_t>(info.type));

			GPUSceneData::SetupConstants(context, gpuSceneData);
		
			constexpr uint32_t workGroupSize = 64;

			context.Dispatch(Math::DivideRoundUp(info.drawCommandCount, workGroupSize), 1, 1);
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
		[=](RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("TaskSubmitSetup");

			context.BindPipeline(pipeline);
			context.SetConstant("countCommandBuffer"_sh, data.countCommandBuffer);
			context.SetConstant("taskCommands"_sh, data.taskCommandsBuffer);
			context.Dispatch(1, 1, 1);
		});
	}
}
