#pragma once

#include "Volt/Asset/Asset.h"

#include <GEM/gem.h>

namespace Volt
{
	class Skeleton;
	class Animation : public Asset
	{
	public:
		struct TRS
		{
			gem::vec3 position;
			gem::quat rotation;
			gem::vec3 scale;
 		};

		struct Frame
		{
			std::vector<gem::mat4> localTransforms;
			std::vector<TRS> localTRS;
		};

		const std::vector<gem::mat4> Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping);
		const std::vector<gem::mat4> SampleCrossfaded(float lerpT, float aNormalizedTime, float aOtherNormalizedTime, Ref<Skeleton> aSkeleton, Ref<Animation> otherAnimation);

		inline const float GetDuration() const { return myDuration; }

		static AssetType GetStaticType() { return AssetType::Animation; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		struct FrameAnimData
		{
			size_t currentFrameIndex = 0;
			size_t nextFrameIndex = 0;
			float deltaTime = 0.f;
		};

		static const gem::mat4 BlendFrames(const Frame& currentFrame, const Frame& nextFrame, const gem::mat4& parentTransform, const size_t jointIndex, const float blendFactor);
		static const FrameAnimData GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime);

		friend class FbxImporter;
		friend class AnimationImporter;

		std::vector<Frame> myFrames;
		uint32_t myFramesPerSecond = 0;
		float myDuration = 0.f;
	};
}