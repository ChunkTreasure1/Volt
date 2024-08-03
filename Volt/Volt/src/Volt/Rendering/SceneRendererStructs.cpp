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

	void GPUSceneData::SetupConstants(RenderContext& context, const GPUSceneData& data)
	{
		context.SetConstant("gpuScene.meshesBuffer"_sh, data.meshesBuffer);
		context.SetConstant("gpuScene.sdfMeshesBuffer"_sh, data.sdfMeshesBuffer);
		context.SetConstant("gpuScene.materialsBuffer"_sh, data.materialsBuffer);
		context.SetConstant("gpuScene.primitiveDrawDataBuffer"_sh, data.primitiveDrawDataBuffer);
		context.SetConstant("gpuScene.sdfPrimitiveDrawDataBuffer"_sh, data.sdfPrimitiveDrawDataBuffer);
		context.SetConstant("gpuScene.bonesBuffer"_sh, data.bonesBuffer);
	}
}
