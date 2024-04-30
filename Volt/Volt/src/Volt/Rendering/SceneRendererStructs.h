#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	enum class VisibilityVisualization
	{
		None = 0,
		ObjectID = 1,
		TriangleID = 2
	};

	struct ExternalImagesData
	{
		RenderGraphResourceHandle black1x1Cube;
		RenderGraphResourceHandle BRDFLuT;
	};

	struct ExternalBuffersData
	{
		RenderGraphResourceHandle drawContextBuffer;
		RenderGraphResourceHandle objectDrawDataBuffer;
		RenderGraphResourceHandle gpuMeshesBuffer;
		RenderGraphResourceHandle gpuMeshletsBuffer;
		RenderGraphResourceHandle gpuSceneBuffer;
	};

	struct EnvironmentTexturesData
	{
		RenderGraphResourceHandle radiance;
		RenderGraphResourceHandle irradiance;
	};

	struct CullObjectsData
	{
		RenderGraphResourceHandle meshletToObjectIdAndOffset;
		RenderGraphResourceHandle meshletCount;

		RenderGraphResourceHandle statisticsBuffer;
	};

	struct CullMeshletsData
	{
		RenderGraphResourceHandle survivingMeshlets;
		RenderGraphResourceHandle survivingMeshletCount;
	};


	struct CullPrimitivesData
	{
		RenderGraphResourceHandle indexBuffer;
		RenderGraphResourceHandle drawCommand;
	};

	struct UniformBuffersData
	{
		RenderGraphResourceHandle viewDataBuffer;
		RenderGraphResourceHandle directionalLightBuffer;
	};

	struct LightBuffersData
	{
		RenderGraphResourceHandle pointLightsBuffer;
		RenderGraphResourceHandle spotLightsBuffer;
	};

	struct PreDepthData
	{
		RenderGraphResourceHandle depth;
		RenderGraphResourceHandle normals;
	};

	struct DirectionalShadowData
	{
		RenderGraphResourceHandle shadowTexture;
		glm::uvec2 renderSize;
	};

	struct VisibilityBufferData
	{
		RenderGraphResourceHandle visibility;
	};

	struct MaterialCountData
	{
		RenderGraphResourceHandle materialCountBuffer;
		RenderGraphResourceHandle materialStartBuffer;
	};

	struct MaterialPixelsData
	{
		RenderGraphResourceHandle pixelCollectionBuffer;
		RenderGraphResourceHandle currentMaterialCountBuffer;
	};

	struct MaterialIndirectArgsData
	{
		RenderGraphResourceHandle materialIndirectArgsBuffer;
	};

	struct GBufferData
	{
		RenderGraphResourceHandle albedo;
		RenderGraphResourceHandle normals;
		RenderGraphResourceHandle material;
		RenderGraphResourceHandle emissive;
	};

	struct ShadingOutputData
	{
		RenderGraphResourceHandle colorOutput;
	};

	struct FinalCopyData
	{
		RenderGraphResourceHandle output;
	};

	struct GTAOOutput
	{
		RenderGraphResourceHandle tempImage;
		RenderGraphResourceHandle outputImage;
	};

	struct GTAOSettings
	{
		float radius = 50.f;
		float radiusMultiplier = 1.457f;
		float falloffRange = 0.615f;
		float finalValuePower = 2.2f;
	};

	struct TestUIData
	{
		RenderGraphResourceHandle outputTextureHandle;
		RenderGraphResourceHandle uiCommandsBufferHandle;
	};
}
