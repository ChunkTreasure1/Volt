#include "vtpch.h"
#include "Volt/Vision/TimelinePlayer.h"

#include "Volt/Vision/Vision.h"

Volt::TimelinePlayer::TimelinePlayer()
{
}

void Volt::TimelinePlayer::Update(const float& deltaTime, Scene* scene)
{
	myCurrentScene = scene;
	if (myIsPlaying)
	{
		PlayTimeline(deltaTime, scene);
	}
}

void Volt::TimelinePlayer::OnRuntimeStart(Scene* scene)
{
	myCurrentScene = scene;
	myIsPlaying = false;
	myCurrentPlaybackTime = 0.f;
}

void Volt::TimelinePlayer::OnRuntimeEnd(Scene* scene)
{
	myCurrentScene = scene;
	myIsPlaying = false;
	myCurrentPlaybackTime = 0.f;

	myCurrentKeyAndTime.clear();
}

void Volt::TimelinePlayer::StartTimeline(const TimelinePreset timelinePreset, Scene* scene)
{
	if (myIsPlaying) { return; }

	scene->GetVision().Initialize();

	myTimelinePreset = timelinePreset;

	myIsPlaying = true;
	myCurrentPlaybackTime = 0.f;

	myEntityStartValues.resize(timelinePreset.myTracks.size(), StartValue());
	for (size_t i = 0; i < timelinePreset.myTracks.size(); i++)
	{
		Entity ent = scene->GetEntityFromUUID(timelinePreset.myTracks[i].targetEntity);

		if (ent)
		{
			myEntityStartValues[i].entID = ent;
			myEntityStartValues[i].Position = ent.GetPosition();
			myEntityStartValues[i].Rotation = ent.GetRotation();
		}
	}

	myCurrentKeyAndTime.clear();
	myCurrentKeyAndTime.resize(myTimelinePreset.myTracks.size(), { 0, 0.f });
}

void Volt::TimelinePlayer::StopTimeline()
{
	myIsPlaying = false;
	myCurrentPlaybackTime = 0.f;

	myCurrentScene->GetVision().Reset();

	myCurrentKeyAndTime.clear();

	for (size_t i = 0; i < myTimelinePreset.myTracks.size(); i++)
	{
		Entity ent = myCurrentScene->GetEntityFromUUID(myTimelinePreset.myTracks[i].targetEntity);

		if (ent)
		{
			ent.SetPosition(myEntityStartValues[i].Position);
			ent.SetRotation(myEntityStartValues[i].Rotation);
		}
	}
}

void Volt::TimelinePlayer::PlayTimeline(const float& deltaTime, Scene* scene)
{
	auto& vision = scene->GetVision();
	vision.Update(deltaTime);
	for (size_t trackIndex = 0; trackIndex < myTimelinePreset.myTracks.size(); trackIndex++)
	{
		const Volt::Track track = myTimelinePreset.myTracks[trackIndex];

		if (track.trackType == TrackType::T_Animation)
		{
#pragma region AnimationTrack

			if (track.keyframes.empty() || track.targetEntity == Entity::NullID() || myCurrentKeyAndTime[trackIndex].first == track.keyframes.size() - 1) { continue; }

			Entity trackEntity = scene->GetEntityFromUUID(track.targetEntity);

			if (myCurrentPlaybackTime <= track.keyframes.at(0).time)
			{
				trackEntity.SetPosition(track.keyframes.at(0).position, true);
				trackEntity.SetRotation(track.keyframes.at(0).rotation, true);
				myCurrentPlaybackTime += deltaTime;
				return;
			}

			const Keyframe& fromKeyframe = track.keyframes[myCurrentKeyAndTime[trackIndex].first];
			const Keyframe& toKeyframe = track.keyframes[myCurrentKeyAndTime[trackIndex].first + 1];

			const float timeDiff = toKeyframe.time - fromKeyframe.time;
			float currentTimePercentage = myCurrentKeyAndTime[trackIndex].second / timeDiff;

			currentTimePercentage = currentTimePercentage * currentTimePercentage * currentTimePercentage * (currentTimePercentage * (6.f * currentTimePercentage - 15.f) + 10.f);

			//Update Entity Transforms
			{
				if (trackEntity.HasComponent<Volt::VisionCameraComponent>())
				{
					const auto& VCamComp = trackEntity.GetComponent<Volt::VisionCameraComponent>();
					if (VCamComp.followId == Entity::NullID())
					{
						trackEntity.SetPosition(glm::mix(fromKeyframe.position, toKeyframe.position, currentTimePercentage));
					}

					if (VCamComp.lookAtId == Entity::NullID())
					{
						trackEntity.SetRotation(glm::slerp(fromKeyframe.rotation, toKeyframe.rotation, currentTimePercentage));
					}
				}
				else
				{
					trackEntity.SetPosition(glm::mix(fromKeyframe.position, toKeyframe.position, currentTimePercentage));
					trackEntity.SetRotation(glm::slerp(fromKeyframe.rotation, toKeyframe.rotation, currentTimePercentage));
				}

			}

			myCurrentKeyAndTime[trackIndex].second += deltaTime;

			if (myCurrentKeyAndTime[trackIndex].second > timeDiff)
			{
				myCurrentKeyAndTime[trackIndex].first++;
				myCurrentKeyAndTime[trackIndex].second = 0;
			}
#pragma endregion
		}
		else if (track.trackType == TrackType::T_Clip)
		{
#pragma region ClipTrack

			if (track.clips.empty() || myCurrentKeyAndTime[trackIndex].first == track.clips.size() - 1) { continue; }

			if (myCurrentPlaybackTime <= track.clips.at(0).startTime)
			{
				if (vision.GetActiveCamera().GetID() != track.clips[0].activeCamera)
				{
					vision.SetActiveCamera(track.clips[0].activeCamera, 0, Volt::eBlendType::None);
					myCurrentPlaybackTime += deltaTime;
					return;
				}
			}

			const Clip& currClip = track.clips[myCurrentKeyAndTime[trackIndex].first];
			const Clip& nextClip = track.clips[myCurrentKeyAndTime[trackIndex].first + 1];

			if (myCurrentPlaybackTime >= nextClip.startTime)
			{
				if (nextClip.startTime < currClip.endTime)
				{
					const float timeDiff = currClip.endTime - nextClip.startTime;
					vision.SetActiveCamera(nextClip.activeCamera, timeDiff, Volt::eBlendType::EaseInAndOut);
				}
				else
				{
					vision.SetActiveCamera(nextClip.activeCamera, 0, Volt::eBlendType::None);
				}
				myCurrentKeyAndTime[trackIndex].first++;
			}

#pragma endregion
		}
	}

	myCurrentPlaybackTime += deltaTime;
	const float timelineMaxLengthTime = myTimelinePreset.maxLength / 30.f;
	if (myCurrentPlaybackTime >= timelineMaxLengthTime)
	{
		Loop();
	}
}

void Volt::TimelinePlayer::Loop()
{
	for (auto& keyAndTime : myCurrentKeyAndTime)
	{
		keyAndTime.first = 0;
		keyAndTime.second = 0.f;
	}
	myCurrentPlaybackTime = 0.f;
}

void Volt::TimelinePlayer::GetPreviewOnTime(TimelinePreset& timelinePreset, const float& timeStamp, Volt::Track& selectedTrack, Scene* scene)
{
	for (size_t trackIndex = 0; trackIndex < timelinePreset.myTracks.size(); trackIndex++)
	{
		const Volt::Track track = timelinePreset.myTracks[trackIndex];

		if (&selectedTrack == &timelinePreset.myTracks[trackIndex] || track.keyframes.empty() || track.targetEntity == Entity::NullID()) { continue; }

		Entity trackEntity = scene->GetEntityFromUUID(track.targetEntity);

		if (timeStamp <= track.keyframes.at(0).time)
		{
			trackEntity.SetPosition(track.keyframes.at(0).position, true);
			trackEntity.SetRotation(track.keyframes.at(0).rotation, true);
			return;
		}

		if (timeStamp >= track.keyframes.at(track.keyframes.size() - 1).time)
		{
			trackEntity.SetPosition(track.keyframes.at(track.keyframes.size() - 1).position, true);
			trackEntity.SetRotation(track.keyframes.at(track.keyframes.size() - 1).rotation, true);
			return;
		}

		for (size_t keyIndex = 0; keyIndex < track.keyframes.size(); keyIndex++)
		{
			if (timeStamp >= track.keyframes[keyIndex].time && timeStamp <= track.keyframes[keyIndex + 1].time)
			{
				const Keyframe& fromKeyframe = track.keyframes[keyIndex];
				const Keyframe& toKeyframe = track.keyframes[keyIndex + 1];

				const float timeDiff = toKeyframe.time - fromKeyframe.time;
				float currentTimePercentage = (timeStamp - fromKeyframe.time) / timeDiff;

				currentTimePercentage = currentTimePercentage * currentTimePercentage * currentTimePercentage * (currentTimePercentage * (6.f * currentTimePercentage - 15.f) + 10.f);

				//Update Entity Transforms
				{
					trackEntity.SetPosition(glm::mix(fromKeyframe.position, toKeyframe.position, currentTimePercentage));
					trackEntity.SetRotation(glm::slerp(fromKeyframe.rotation, toKeyframe.rotation, currentTimePercentage));
				}
			}
		}
	}
}
