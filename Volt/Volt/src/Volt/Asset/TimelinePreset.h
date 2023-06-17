#pragma once
#include "Volt/Asset/Asset.h"
#include "Wire/Wire.h"

namespace Volt
{
	enum class TrackObjectType : uint32_t
	{
		Clip,
		Keyframe
	};

	enum class TrackType : uint32_t
	{
		T_Animation,
		T_Clip
	};

	enum class KeyframeType : uint32_t
	{
		KF_Position,
		KF_Rotation,
		KF_Scale
	};

	struct Keyframe
	{
		Keyframe() = default;
		Keyframe(KeyframeType type) { keyframeType = type; };

		KeyframeType keyframeType = KeyframeType::KF_Position;

		float timelineXPos = 0.f;

		float time = 0.f;
		int frame = 0;

		glm::vec3 position = 0;
		glm::quat rotation = glm::quat();
	};

	struct Clip
	{
		float startTime = 0.f;
		float endTime = 0.f;

		Wire::EntityId activeCamera = Wire::NullID;
	};

	struct Track
	{
		Track() = default;
		Track(TrackType type) { trackType = type; }

		TrackType trackType = TrackType::T_Animation;

		Wire::EntityId targetEntity = Wire::NullID;

		std::vector<Keyframe> keyframes = std::vector<Keyframe>();
		std::vector<Clip> clips = std::vector<Clip>();
	};

	class TimelinePreset : public Asset
	{
	public:
		std::vector<Track> myTracks = std::vector<Track>();

		int maxLength = 300;

		static AssetType GetStaticType() { return AssetType::Timeline; }
		virtual AssetType GetType() override { return GetStaticType(); }
	};
}
