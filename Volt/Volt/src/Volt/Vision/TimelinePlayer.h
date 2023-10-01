#pragma once
#include "Volt/Asset/TimelinePreset.h"

#include <entt.hpp>

namespace Volt
{
	class Scene;
	class Entity;

	struct StartValue
	{
		entt::entity entID;

		glm::vec3 Position = glm::vec3(0);
		glm::quat Rotation = glm::quat();
	};

	class TimelinePlayer
	{
	public:
		TimelinePlayer();
		~TimelinePlayer() { myCurrentScene = nullptr; };

		void Update(const float& deltaTime, Scene* scene);
		void OnRuntimeStart(Scene* scene);
		void OnRuntimeEnd(Scene* scene);

		void StartTimeline(const TimelinePreset timelinePreset, Scene* scene);
		void StopTimeline();
		void GetPreviewOnTime(TimelinePreset& timelinePreset, const float& timeStamp, Volt::Track& selectedTrack, Scene* scene);

	private:

		void Loop();
		void PlayTimeline(const float& deltaTime, Scene* scene);
		
		Scene* myCurrentScene = nullptr;

		std::vector<StartValue> myEntityStartValues;
		std::vector<std::pair<int, float>> myCurrentKeyAndTime;
		float myCurrentPlaybackTime = 0.f;
		bool myIsPlaying = false;
		TimelinePreset myTimelinePreset;
	};
}
