#pragma once

#include "Volt/Asset/Asset.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Skeleton;
	class Animation : public Asset
	{
	public:
		struct TRS
		{
			glm::vec3 position = { 0.f };
			glm::quat rotation = { 1.f, 0.f, 0.f, 0.f };
			glm::vec3 scale = { 1.f };
		};

		struct Pose
		{
			std::vector<TRS> localTRS;
		};

		const std::vector<glm::mat4> Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping);
		const std::vector<glm::mat4> Sample(uint32_t frameIndex, Ref<Skeleton> aSkeleton);

		const std::vector<TRS> SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping, float speed = 1.f) const;
		const bool IsAtEnd(float startTime, float speed);
		const bool HasPassedTime(float startTime, float speed, float time);

		const uint32_t GetFrameFromStartTime(float startTime, float speed);

		inline const float GetDuration() const { return myDuration; }
		inline const size_t GetFrameCount() const { return myFrames.size(); }

		static AssetType GetStaticType() { return AssetType::Animation; }
		AssetType GetType() override { return GetStaticType(); };

	private:
		struct PoseData
		{
			size_t currentFrameIndex = 0;
			size_t nextFrameIndex = 0;
			float deltaTime = 0.f;
		};

		static const glm::mat4 BlendFrames(const Pose& currentFrame, const Pose& nextFrame, const glm::mat4& parentTransform, const size_t jointIndex, const float blendFactor);
		static const PoseData GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime);

		friend class FbxImporter;
		friend class AnimationImporter;

		std::vector<Pose> myFrames;

		uint32_t myFramesPerSecond = 0;
		float myDuration = 0.f;
	};
}
