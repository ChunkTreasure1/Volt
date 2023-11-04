#pragma once

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

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
		RenderGraphResourceHandle outputImage;
	};

	struct ExternalBuffersData
	{
		RenderGraphResourceHandle indirectCommandsBuffer;
		RenderGraphResourceHandle indirectCountsBuffer;

		RenderGraphResourceHandle instanceOffsetToObjectIDBuffer;

		RenderGraphResourceHandle drawContextBuffer;
	};

	struct UniformBuffersData
	{
		RenderGraphResourceHandle cameraDataBuffer;
	};

	struct PreDepthData
	{
		RenderGraphResourceHandle depth;
		RenderGraphResourceHandle normals;
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
		RenderGraphResourceHandle materialEmissive;
		RenderGraphResourceHandle normalEmissive;
	};
}
