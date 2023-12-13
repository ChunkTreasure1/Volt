#include "vtpch.h"

#include "TimelineImporter.h"
#include "Volt/Asset/TimelinePreset.h"

#include "Volt/Log/Log.h"
#include "Volt/Project/ProjectManager.h"

#include <yaml-cpp/yaml.h>
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

#include "Volt/Scene/Entity.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	bool TimelineImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		asset = CreateRef<TimelinePreset>();
		Ref<TimelinePreset> preset = std::reinterpret_pointer_cast<TimelinePreset>(asset);

		std::stringstream sstream;
		sstream << file.rdbuf();

		YAML::Node root = YAML::Load(sstream.str());

		VT_DESERIALIZE_PROPERTY(maxLength, preset->maxLength, root, 0);

		for (auto track : root["Tracks"])
		{
			Volt::Track newTrack = Volt::Track();

			VT_DESERIALIZE_PROPERTY(trackType, newTrack.trackType, track, TrackType::T_Animation);

			if (newTrack.trackType == TrackType::T_Animation)
			{
				VT_DESERIALIZE_PROPERTY(targetEntId, newTrack.targetEntity, track, Entity::NullID());

				for (auto keyFrame : track["Keyframes"])
				{
					Volt::Keyframe newKeyFrame = Volt::Keyframe();

					VT_DESERIALIZE_PROPERTY(time, newKeyFrame.time, keyFrame, 0.f);
					VT_DESERIALIZE_PROPERTY(frame, newKeyFrame.frame, keyFrame, 0);
					VT_DESERIALIZE_PROPERTY(timelineXPos, newKeyFrame.timelineXPos, keyFrame, 0.f);
					VT_DESERIALIZE_PROPERTY(position, newKeyFrame.position, keyFrame, glm::vec3(0.f));
					VT_DESERIALIZE_PROPERTY(rotation, newKeyFrame.rotation, keyFrame, glm::quat());

					newTrack.keyframes.emplace_back(newKeyFrame);
				}
			}
			else if (newTrack.trackType == TrackType::T_Clip)
			{
				for (auto clip : track["Clips"])
				{
					Volt::Clip newClip = Volt::Clip();

					VT_DESERIALIZE_PROPERTY(clipCamera, newClip.activeCamera, clip, Entity::NullID());
					VT_DESERIALIZE_PROPERTY(startTime, newClip.startTime, clip, 0.f);
					VT_DESERIALIZE_PROPERTY(endTime, newClip.endTime, clip, 0.f);

					newTrack.clips.emplace_back(newClip);
				}
			}
			preset->myTracks.emplace_back(newTrack);
		}

		return true;
	}

	void TimelineImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<TimelinePreset> preset = std::reinterpret_pointer_cast<TimelinePreset>(asset);

		for (auto& track : preset->myTracks)
		{
			std::sort(track.keyframes.begin(), track.keyframes.end(), [](const Volt::Keyframe& lhsKeyFrame, const Volt::Keyframe& rhsKeyFrame) { return lhsKeyFrame.time < rhsKeyFrame.time; });
		}

		YAML::Emitter out;
		out << YAML::BeginMap;

		VT_SERIALIZE_PROPERTY(maxLength, preset->maxLength, out);

		out << YAML::Key << "Tracks" << YAML::BeginSeq;
		{
			for (const auto& track : preset->myTracks)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Track" << YAML::Value << "";
				{
					if (track.trackType == TrackType::T_Animation)
					{
						VT_SERIALIZE_PROPERTY(trackType, track.trackType, out);
						VT_SERIALIZE_PROPERTY(targetEntId, track.targetEntity, out);

						out << YAML::Key << "Keyframes" << YAML::BeginSeq;
						for (const auto& key : track.keyframes)
						{
							out << YAML::BeginMap;
							out << YAML::Key << "Frame" << YAML::Value << "";
							{
								VT_SERIALIZE_PROPERTY(time, key.time, out);
								VT_SERIALIZE_PROPERTY(frame, key.frame, out);
								VT_SERIALIZE_PROPERTY(timelineXPos, key.timelineXPos, out);
								VT_SERIALIZE_PROPERTY(position, key.position, out);
								VT_SERIALIZE_PROPERTY(rotation, key.rotation, out);
							}
							out << YAML::EndMap;
						}
						out << YAML::EndSeq;
					}
					else if (track.trackType == TrackType::T_Clip)
					{
						VT_SERIALIZE_PROPERTY(trackType, track.trackType, out);

						out << YAML::Key << "Clips" << YAML::BeginSeq;
						for (const auto& clip : track.clips)
						{
							out << YAML::BeginMap;
							out << YAML::Key << "Clip" << YAML::Value << "";
							{
								VT_SERIALIZE_PROPERTY(clipCamera, clip.activeCamera, out);
								VT_SERIALIZE_PROPERTY(startTime, clip.startTime, out);
								VT_SERIALIZE_PROPERTY(endTime, clip.endTime, out);
							}
							out << YAML::EndMap;
						}
						out << YAML::EndSeq;
					}
				}
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
		fout << out.c_str();
		fout.close();
	}
}
