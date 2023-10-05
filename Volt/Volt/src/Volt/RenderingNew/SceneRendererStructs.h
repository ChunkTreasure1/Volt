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

		RenderGraphResourceHandle drawToInstanceOffsetBuffer;
		RenderGraphResourceHandle instanceOffsetToObjectIDBuffer;
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

	struct GBufferData
	{
		RenderGraphResourceHandle albedo;
		RenderGraphResourceHandle materialEmissive;
		RenderGraphResourceHandle normalEmissive;
	};
}
