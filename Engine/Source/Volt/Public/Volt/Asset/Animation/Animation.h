#pragma once

#include "Volt/Asset/AssetTypes.h"

#include <AssetSystem/Asset.h>
#include <AssetSystem/AssetFactory.h>

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

#include <glm/glm.hpp>

namespace Volt
{
	class Skeleton;
	class Animation : public Asset
	{
	public:
		struct TRS
		{
			glm::vec3 translation = { 0.f };
			glm::quat rotation = { 1.f, 0.f, 0.f, 0.f };
			glm::vec3 scale = { 1.f };

			static void Serialize(BinaryStreamWriter& streamWriter, const TRS& data)
			{
				streamWriter.Write(data.translation);
				streamWriter.Write(data.rotation);
				streamWriter.Write(data.scale);
			}

			static void Deserialize(BinaryStreamReader& streamReader, TRS& outData)
			{
				streamReader.Read(outData.translation);
				streamReader.Read(outData.rotation);
				streamReader.Read(outData.scale);
			}
		};

		struct Pose
		{
			Vector<TRS> localTRS;

			static void Serialize(BinaryStreamWriter& streamWriter, const Pose& data)
			{
				streamWriter.WriteRaw(data.localTRS);
			}

			static void Deserialize(BinaryStreamReader& streamReader, Pose& outData)
			{
				streamReader.ReadRaw(outData.localTRS);
			}
		};

		struct Event
		{
			uint32_t frame;
			std::string name;

			static void Serialize(BinaryStreamWriter& streamWriter, const Event& data)
			{
				streamWriter.Write(data.frame);
				streamWriter.Write(data.name);
			}

			static void Deserialize(BinaryStreamReader& streamReader, Event& outData)
			{
				streamReader.Read(outData.frame);
				streamReader.Read(outData.name);
			}
		};

		const Vector<glm::mat4> SampleStartTime(float aStartTime, Ref<Skeleton> aSkeleton, bool looping);
		const Vector<glm::mat4> Sample(float samplePercent, Ref<Skeleton> skeleton, bool looping);
		const Vector<glm::mat4> Sample(uint32_t frameIndex, Ref<Skeleton> aSkeleton);

		static Vector<glm::mat4> LocalPoseToGlobalMatrices(const Pose& localPose, Ref<Skeleton> aSkeleton);
		static void BlendPoseWith(Pose& target, const Pose& poseToBlendWith, float blendFactor);
		static Pose GetBlendedPose(const Pose& target, const Pose& poseToBlendWith, float blendFactor);

		const Vector<TRS> SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping, float speed = 1.f) const;
		const bool IsAtEnd(float startTime, float speed);
		const bool HasPassedTime(float startTime, float speed, float time);

		const uint32_t GetFrameFromStartTime(float startTime, float speed);
		const float GetNormalizedCurrentTimeFromStartTime(float startTime, float speed, bool looping);

		inline const float GetDuration() const { return m_duration; }
		inline const size_t GetFrameCount() const { return m_frames.size(); }
		inline const uint32_t GetFramesPerSecond() const { return m_framesPerSecond; }

		void AddEvent(const std::string& eventName, uint32_t frame);
		void RemoveEvent(const std::string& eventName, uint32_t frame);
		inline const bool HasEvents() const { return !m_events.empty(); }
		inline const Vector<Event>& GetEvents() const { return m_events; }

		static AssetType GetStaticType() { return AssetTypes::Animation; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

	private:
		struct PoseData
		{
			size_t currentFrameIndex = 0;
			size_t nextFrameIndex = 0;
			float deltaTime = 0.f;
		};

		static const PoseData GetFrameDataFromAnimation(Animation& animation, const float aNormalizedTime);

		friend class FbxSourceImporter;
		friend class AnimationImporter;
		friend class AnimationSerializer;

		Vector<Pose> m_frames;
		Vector<Event> m_events;

		uint32_t m_framesPerSecond = 0;
		float m_duration = 0.f;
	};
}
