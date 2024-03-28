#pragma once

#include "Volt/Asset/Asset.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"
#include "Volt/Utility/FileIO/BinaryStreamReader.h"

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

			static void Serialize(BinaryStreamWriter& streamWriter, const TRS& data)
			{
				streamWriter.Write(data.position);
				streamWriter.Write(data.rotation);
				streamWriter.Write(data.scale);
			}

			static void Deserialize(BinaryStreamReader& streamReader, TRS& outData)
			{
				streamReader.Read(outData.position);
				streamReader.Read(outData.rotation);
				streamReader.Read(outData.scale);
			}
		};

		struct Pose
		{
			std::vector<TRS> localTRS;

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
		};

		const std::vector<glm::mat4> Sample(float aStartTime, Ref<Skeleton> aSkeleton, bool looping);
		const std::vector<glm::mat4> Sample(uint32_t frameIndex, Ref<Skeleton> aSkeleton);

		const std::vector<TRS> SampleTRS(float aStartTime, Ref<Skeleton> aSkeleton, bool looping, float speed = 1.f) const;
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
		inline const std::vector<Event>& GetEvents() const { return m_events; }

		static AssetType GetStaticType() { return AssetType::Animation; }
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

		friend class FbxImporter;
		friend class AnimationImporter;
		friend class AnimationSerializer;

		std::vector<Pose> m_frames;
		std::vector<Event> m_events;

		uint32_t m_framesPerSecond = 0;
		float m_duration = 0.f;
	};
}
