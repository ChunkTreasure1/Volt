#pragma once

#include "Volt/Rendering/FrameGraph/FrameGraphResourceHandle.h"

namespace Volt
{
	class CommandBuffer;
	class ComputePipeline;
	class Image2D;
	class GlobalDescriptorSet;
	class Camera;
	class FrameGraph;

	class TAATechnique
	{
	public:
		TAATechnique(Ref<Camera> camera, const gem::mat4& reprojectionMatrix, const gem::vec2ui renderSize, uint64_t frameIndex, const gem::vec2& jitterDelta);

		void AddGenerateMotionVectorsPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<GlobalDescriptorSet> rendererBuffersSet, FrameGraphResourceHandle srcDepth);
		void AddTAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<Image2D> colorHistory, FrameGraphResourceHandle srcDepthHandle);
		void AddTAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		struct TAAConstants
		{
			gem::vec4 resolution; //width, height, 1/width, 1/height
			gem::vec2 jitter;

			uint32_t frameNumber;
			uint32_t debugFlags;
			float lerpMul;
			float lerpPow;
			float varClipGammaMin;
			float varClipGammaMax;
			float preExposureNewOverOld;
			float Padding0;
			float Padding1;
			float Padding2;

		} myConstants;

		Ref<Camera> myCamera;
		
		gem::vec2 myJitterDelta;
		gem::vec2ui myRenderSize;
		gem::mat4 myReprojectionMatrix;

		uint64_t myFrameIndex = 0;
	};
}
