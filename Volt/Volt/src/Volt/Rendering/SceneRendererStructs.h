#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"

namespace Volt
{
	class Camera;
	class RenderContext;
	class RenderGraphPassResources;
	class RenderGraph;
	class RenderGraph::Builder;

	enum class VisibilityVisualization
	{
		None = 0,
		ObjectID = 1,
		TriangleID = 2
	};

	struct RenderData
	{
		Ref<Camera> camera;
		glm::uvec2 renderSize;
	};

	struct PreviousFrameData
	{
		glm::mat4 viewProjection = glm::identity<glm::mat4>();
		glm::vec2 jitter = 0.f;
	};

	struct ExternalImagesData
	{
		RenderGraphImage2DHandle black1x1Cube;
		RenderGraphImage2DHandle BRDFLuT;
	};

	struct EnvironmentTexturesData
	{
		RenderGraphImage2DHandle radiance;
		RenderGraphImage2DHandle irradiance;
	};

	struct UniformBuffersData
	{
		RenderGraphUniformBufferHandle viewDataBuffer;
		RenderGraphUniformBufferHandle directionalLightBuffer;
	};

	struct GPUSceneData
	{
		RenderGraphBufferHandle meshesBuffer;
		RenderGraphBufferHandle sdfMeshesBuffer;
		RenderGraphBufferHandle materialsBuffer;
		RenderGraphBufferHandle primitiveDrawDataBuffer;
		RenderGraphBufferHandle sdfPrimitiveDrawDataBuffer;
		RenderGraphBufferHandle bonesBuffer;

		static void SetupInputs(RenderGraph::Builder& builder, const GPUSceneData& data);
		static void SetupConstants(RenderContext& context, const GPUSceneData& data);
	};

	struct LightBuffersData
	{
		RenderGraphBufferHandle pointLightsBuffer;
		RenderGraphBufferHandle spotLightsBuffer;
	};

	struct PreDepthData
	{
		RenderGraphImage2DHandle depth;
		RenderGraphImage2DHandle normals;
	};

	struct DirectionalShadowData
	{
		RenderGraphImage2DHandle shadowTexture;
		glm::uvec2 renderSize;
	};

	struct VisibilityBufferData
	{
		RenderGraphImage2DHandle visibility;
	};

	struct MaterialCountData
	{
		RenderGraphBufferHandle materialCountBuffer;
		RenderGraphBufferHandle materialStartBuffer;
	};

	struct MaterialPixelsData
	{
		RenderGraphBufferHandle pixelCollectionBuffer;
		RenderGraphBufferHandle currentMaterialCountBuffer;
	};

	struct MaterialIndirectArgsData
	{
		RenderGraphBufferHandle materialIndirectArgsBuffer;
	};

	struct GBufferData
	{
		RenderGraphImage2DHandle albedo;
		RenderGraphImage2DHandle normals;
		RenderGraphImage2DHandle material;
		RenderGraphImage2DHandle emissive;
	};

	struct ShadingOutputData
	{
		RenderGraphImage2DHandle colorOutput;
	};

	struct FXAAOutputData
	{
		RenderGraphImage2DHandle output;
	};

	struct FinalCopyData
	{
		RenderGraphImage2DHandle output;
	};

	struct GTAOOutput
	{
		RenderGraphImage2DHandle tempImage;
		RenderGraphImage2DHandle outputImage;
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
		RenderGraphImage2DHandle outputTextureHandle;
		RenderGraphImage2DHandle uiCommandsBufferHandle;
	};
}
