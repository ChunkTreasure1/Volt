#pragma once

#include "Volt/Rendering/FrameGraph/FrameGraphResourceHandle.h"

namespace Volt
{
	struct GBufferData
	{
		FrameGraphResourceHandle albedo;
		FrameGraphResourceHandle materialEmissive;
		FrameGraphResourceHandle normalEmissive;
		FrameGraphResourceHandle depth;
	};

	struct DeferredShadingData
	{
	};

	struct ForwardData
	{
	};

	struct ForwardTransparentData
	{
		FrameGraphResourceHandle accumulation;
		FrameGraphResourceHandle revealage;
	};
	
	struct ForwardSSSData
	{
		FrameGraphResourceHandle diffuse;
	};

	struct PreDepthData
	{
		FrameGraphResourceHandle viewNormals;
		FrameGraphResourceHandle motionVectors;
		FrameGraphResourceHandle preDepth;
	};

	struct IDData
	{
		FrameGraphResourceHandle id;
		FrameGraphResourceHandle depth;
	};

	struct DirectionalShadowData
	{
		FrameGraphResourceHandle shadowMap;

		gem::vec2ui size = { 0 };
	};

	struct PreethamSkyData
	{
		FrameGraphResourceHandle skybox;
	};

	struct SkyboxData
	{
		FrameGraphResourceHandle outputImage;
		FrameGraphResourceHandle radianceImage;
		FrameGraphResourceHandle irradianceImage;
	};

	struct LuminosityData
	{
		FrameGraphResourceHandle luminosityImage;
	};

	struct BloomDownsampleData
	{
		FrameGraphResourceHandle downsampledImage;
	};

	struct GTAOOutput
	{
		FrameGraphResourceHandle tempImageHandle;
		FrameGraphResourceHandle outputImage;
	};

	struct MotionVectorData
	{
		FrameGraphResourceHandle motionVectors;
		FrameGraphResourceHandle currentDepth;
	};

	struct SpotLightShadowData
	{
		std::vector<uint32_t> spotLightIndices;
		std::vector<FrameGraphResourceHandle> spotLightShadows;
	};

	struct PointLightShadowData
	{
		std::vector<uint32_t> pointLightIndices;
		std::vector<FrameGraphResourceHandle> pointLightShadows;
	};

	struct SSRData
	{
		FrameGraphResourceHandle ssrUVs;
	};

	struct DepthReductionData
	{
		FrameGraphResourceHandle reducedDepth;
	};

	struct VolumetricFogData
	{
		FrameGraphResourceHandle rayMarched;
	};

	struct SSSBlurData
	{
		FrameGraphResourceHandle blurOne;
		FrameGraphResourceHandle blurTwo;
	};
}
