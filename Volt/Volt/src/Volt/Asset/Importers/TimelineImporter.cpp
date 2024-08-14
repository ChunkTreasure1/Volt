#include "vtpch.h"

#include "TimelineImporter.h"
#include "Volt/Asset/TimelinePreset.h"

#include "Volt/Project/ProjectManager.h"

#include <yaml-cpp/yaml.h>
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Scene/Entity.h"

#include <AssetSystem/AssetManager.h>

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

namespace Volt
{
	bool TimelineImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_LOG(Error, "Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = CreateRef<TimelinePreset>();
		Ref<TimelinePreset> preset = std::reinterpret_pointer_cast<TimelinePreset>(asset);

		preset->maxLength = streamReader.ReadAtKey("maxLength", 0);

		streamReader.ForEach("Tracks", [&]() 
		{
			Volt::Track newTrack = Volt::Track();
			
			newTrack.trackType = streamReader.ReadAtKey("trackType", TrackType::T_Animation);

			if (newTrack.trackType == TrackType::T_Animation)
			{
				newTrack.targetEntity = streamReader.ReadAtKey("targetEntId", Entity::NullID());

				streamReader.ForEach("Keyframes", [&]() 
				{
					Volt::Keyframe newKeyFrame = Volt::Keyframe();

					newKeyFrame.time = streamReader.ReadAtKey("time", 0.f);
					newKeyFrame.frame = streamReader.ReadAtKey("frame", 0);
					newKeyFrame.timelineXPos = streamReader.ReadAtKey("timelineXPos", 0.f);
					newKeyFrame.position = streamReader.ReadAtKey("position", glm::vec3(0.f));
					newKeyFrame.rotation = streamReader.ReadAtKey("rotation", glm::identity<glm::quat>());

					newTrack.keyframes.emplace_back(newKeyFrame);
				});
			}
			else if (newTrack.trackType == TrackType::T_Clip)
			{
				streamReader.ForEach("Clips", [&]() 
				{
					Volt::Clip newClip = Volt::Clip();
					
					newClip.activeCamera = streamReader.ReadAtKey("clipCamera", Entity::NullID());
					newClip.startTime = streamReader.ReadAtKey("startTime", 0.f);
					newClip.endTime = streamReader.ReadAtKey("endTime", 0.f);

					newTrack.clips.emplace_back(newClip);
				});
			}

			preset->myTracks.emplace_back(newTrack);
		});

		return true;
	}

	void TimelineImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<TimelinePreset> preset = std::reinterpret_pointer_cast<TimelinePreset>(asset);

		for (auto& track : preset->myTracks)
		{
			std::sort(track.keyframes.begin(), track.keyframes.end(), [](const Volt::Keyframe& lhsKeyFrame, const Volt::Keyframe& rhsKeyFrame) { return lhsKeyFrame.time < rhsKeyFrame.time; });
		}

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };
		
		streamWriter.BeginMap();
		
		streamWriter.SetKey("maxLength", preset->maxLength);
		
		streamWriter.BeginSequence("Tracks");
		for (const auto& track : preset->myTracks)
		{
			streamWriter.BeginMap();

			if (track.trackType == TrackType::T_Animation)
			{
				streamWriter.SetKey("trackType", track.trackType);
				streamWriter.SetKey("targetEntId", track.targetEntity);

				streamWriter.BeginSequence("Keyframes");
				for (const auto& key : track.keyframes)
				{
					streamWriter.BeginMap();

					streamWriter.SetKey("time", key.time);
					streamWriter.SetKey("frame", key.frame);
					streamWriter.SetKey("timelineXPos", key.timelineXPos);
					streamWriter.SetKey("position", key.position);
					streamWriter.SetKey("rotation", key.rotation);

					streamWriter.EndMap();
				}
				streamWriter.EndSequence();
			}
			else if (track.trackType == TrackType::T_Clip)
			{
				streamWriter.SetKey("trackType", track.trackType);

				streamWriter.BeginSequence("Clips");
				for (const auto& clip : track.clips)
				{
					streamWriter.SetKey("clipCamera", clip.activeCamera);
					streamWriter.SetKey("startTime", clip.startTime);
					streamWriter.SetKey("endTime", clip.endTime);
				}
				streamWriter.EndSequence();
			}

			streamWriter.EndMap();
		}
		streamWriter.EndSequence();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
