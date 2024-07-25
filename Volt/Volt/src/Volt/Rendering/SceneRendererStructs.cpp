#include "vtpch.h"
#include "SceneRendererStructs.h"

namespace Volt
{
	void GPUSceneData::SetupInputs(RenderGraph::Builder& builder, const GPUSceneData& data)
	{
		builder.ReadResource(data.meshesBuffer);
		builder.ReadResource(data.sdfMeshesBuffer);
		builder.ReadResource(data.materialsBuffer);
		builder.ReadResource(data.primitiveDrawDataBuffer);
		builder.ReadResource(data.sdfPrimitiveDrawDataBuffer);
		builder.ReadResource(data.bonesBuffer);
	}

	void GPUSceneData::SetupConstants(RenderContext& context, const RenderGraphPassResources& resources, const GPUSceneData& data)
	{
		context.SetConstant("gpuScene.meshesBuffer"_sh, resources.GetBuffer(data.meshesBuffer));
		context.SetConstant("gpuScene.sdfMeshesBuffer"_sh, resources.GetBuffer(data.sdfMeshesBuffer));
		context.SetConstant("gpuScene.materialsBuffer"_sh, resources.GetBuffer(data.materialsBuffer));
		context.SetConstant("gpuScene.primitiveDrawDataBuffer"_sh, resources.GetBuffer(data.primitiveDrawDataBuffer));
		context.SetConstant("gpuScene.sdfPrimitiveDrawDataBuffer"_sh, resources.GetBuffer(data.sdfPrimitiveDrawDataBuffer));
		context.SetConstant("gpuScene.bonesBuffer"_sh, resources.GetBuffer(data.bonesBuffer));
	}
}
