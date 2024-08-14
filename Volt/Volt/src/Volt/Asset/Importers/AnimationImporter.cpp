#include "vtpch.h"
#include "AnimationImporter.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	struct AnimEventsHeader
	{
		uint32_t eventCount;
	};

	struct AnimEventHeader
	{
		uint32_t totalSize;
		uint32_t stringSize;
	};

	bool AnimationImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Animation>();
		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream input(filePath, std::ios::binary | std::ios::in);
		if (!input.is_open())
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		Vector<uint8_t> totalData;
		totalData.resize(input.seekg(0, std::ios::end).tellg());
		input.seekg(0, std::ios::beg);
		input.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		input.close();

		if (totalData.size() < sizeof(AnimationHeader))
		{
			VT_LOG(Error, "File is smaller than header!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		AnimationHeader header = *(AnimationHeader*)&totalData[0];
		size_t offset = sizeof(AnimationHeader);

		Vector<Animation::Pose> frames;
		frames.resize(header.frameCount);

		for (uint32_t i = 0; i < header.frameCount; i++)
		{
			auto& currFrame = frames[i];
			currFrame.localTRS.resize(header.perFrameTransformCount);

			memcpy_s(currFrame.localTRS.data(), header.perFrameTransformCount * sizeof(Animation::TRS), &totalData[offset], header.perFrameTransformCount * sizeof(Animation::TRS));
			offset += header.perFrameTransformCount * sizeof(Animation::TRS);
		}

		if (offset + sizeof(AnimEventsHeader) < totalData.size())
		{
			AnimEventsHeader eventsHeader = *(AnimEventsHeader*)&totalData[offset];
			offset += sizeof(AnimEventsHeader);

			for (uint32_t i = 0; i < eventsHeader.eventCount; i++)
			{
				AnimEventHeader eHeader = *(AnimEventHeader*)&totalData[offset];
				offset += sizeof(AnimEventHeader);

				auto& animEvent = animation->m_events.emplace_back();
				animEvent.name.resize(eHeader.stringSize);

				animEvent.frame = *(uint32_t*)&totalData[offset];
				offset += sizeof(uint32_t);

				memcpy_s(animEvent.name.data(), eHeader.stringSize, &totalData[offset], eHeader.stringSize);
				offset += eHeader.stringSize;
			}
		}

		animation->m_frames = frames;
		animation->m_duration = header.duration;
		animation->m_framesPerSecond = header.framesPerSecond;
		return true;
	}

	void AnimationImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(asset);

		Vector<uint8_t> outData;

		AnimationHeader header{};
		header.perFrameTransformCount = (uint32_t)animation->m_frames.front().localTRS.size();
		header.frameCount = (uint32_t)animation->m_frames.size();
		header.duration = animation->m_duration;
		header.framesPerSecond = animation->m_framesPerSecond;

		AnimEventsHeader eventsHeader{};
		eventsHeader.eventCount = static_cast<uint32_t>(animation->m_events.size());

		outData.resize(sizeof(AnimationHeader) + header.frameCount * header.perFrameTransformCount * sizeof(Animation::TRS));

		size_t offset = 0;
		memcpy_s(&outData[0], sizeof(AnimationHeader), &header, sizeof(AnimationHeader));
		offset += sizeof(AnimationHeader);

		// Copy matrices per frame
		for (const auto& frame : animation->m_frames)
		{
			memcpy_s(&outData[offset], header.perFrameTransformCount * sizeof(Animation::TRS), frame.localTRS.data(), header.perFrameTransformCount * sizeof(Animation::TRS));
			offset += header.perFrameTransformCount * sizeof(Animation::TRS);
		}

		outData.resize(outData.size() + sizeof(AnimEventsHeader));

		memcpy_s(&outData[offset], sizeof(AnimEventsHeader), &eventsHeader, sizeof(AnimEventsHeader));
		offset += sizeof(AnimEventsHeader);

		for (const auto& animEvent : animation->m_events)
		{
			AnimEventHeader eHeader{};
			eHeader.stringSize = static_cast<uint32_t>(animEvent.name.size());
			eHeader.totalSize = eHeader.stringSize + sizeof(animEvent.frame);

			outData.resize(outData.size() + eHeader.totalSize + sizeof(AnimEventHeader));

			memcpy_s(&outData[offset], sizeof(AnimEventHeader), &eHeader, sizeof(AnimEventHeader));
			offset += sizeof(AnimEventHeader);

			memcpy_s(&outData[offset], sizeof(uint32_t), &animEvent.frame, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			memcpy_s(&outData[offset], eHeader.stringSize, animEvent.name.data(), animEvent.name.size());
			offset += animEvent.name.size();
		}

		std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath), std::ios::binary | std::ios::out);
		fout.write((char*)outData.data(), outData.size());
		fout.close();
	}
}
