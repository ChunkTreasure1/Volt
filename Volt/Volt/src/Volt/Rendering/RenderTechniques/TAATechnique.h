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
		TAATechnique(Ref<Camera> camera, const glm::mat4& reprojectionMatrix, const glm::uvec2 renderSize, uint64_t frameIndex, const glm::vec2& jitterDelta);

		void AddGenerateMotionVectorsPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<GlobalDescriptorSet> rendererBuffersSet, FrameGraphResourceHandle srcDepth);
		void AddTAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<Image2D> colorHistory, FrameGraphResourceHandle srcDepthHandle);
		void AddTAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		struct TAAConstants
		{
			glm::vec4 resolution; //width, height, 1/width, 1/height
			glm::vec2 jitter;

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
		
		glm::vec2 myJitterDelta;
		glm::uvec2 myRenderSize;
		glm::mat4 myReprojectionMatrix;

		uint64_t myFrameIndex = 0;
	};
}
