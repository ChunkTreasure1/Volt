#pragma once

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
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

	struct GBufferData
	{
		RenderGraphResourceHandle albedo;
		RenderGraphResourceHandle materialEmissive;
		RenderGraphResourceHandle normalEmissive;
	};
}
