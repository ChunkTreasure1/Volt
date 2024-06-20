#include "vtpch.h"
#include "SceneRendererStructs.h"

namespace Volt
{
	void GPUSceneData::SetupInputs(RenderGraph::Builder& builder, const GPUSceneData& data)
	{
		builder.ReadResource(data.meshesBuffer);
		builder.ReadResource(data.materialsBuffer);
		builder.ReadResource(data.objectDrawDataBuffer);
		builder.ReadResource(data.meshletsBuffer);
		builder.ReadResource(data.bonesBuffer);
	}

	void GPUSceneData::SetupConstants(RenderContext& context, const RenderGraphPassResources& resources, const GPUSceneData& data)
	{
		context.SetConstant("gpuScene.meshesBuffer"_sh, resources.GetBuffer(data.meshesBuffer));
		context.SetConstant("gpuScene.materialsBuffer"_sh, resources.GetBuffer(data.materialsBuffer));
		context.SetConstant("gpuScene.objectDrawDataBuffer"_sh, resources.GetBuffer(data.objectDrawDataBuffer));
		context.SetConstant("gpuScene.meshletsBuffer"_sh, resources.GetBuffer(data.meshletsBuffer));
		context.SetConstant("gpuScene.bonesBuffer"_sh, resources.GetBuffer(data.bonesBuffer));
	}
}
