#include "vtpch.h"
#include "AnimationImporter.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Asset/Animation/Animation.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	bool AnimationImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Animation>();
		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(asset);

		const auto filePath = AssetManager::GetContextPath(path) / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream input(filePath, std::ios::binary | std::ios::in);
		if (!input.is_open())
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::vector<uint8_t> totalData;
		totalData.resize(input.seekg(0, std::ios::end).tellg());
		input.seekg(0, std::ios::beg);
		input.read(reinterpret_cast<char*>(totalData.data()), totalData.size());
		input.close();

		if (totalData.size() < sizeof(AnimationHeader))
		{
			VT_CORE_ERROR("File is smaller than header!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		AnimationHeader header = *(AnimationHeader*)&totalData[0];
		size_t offset = sizeof(AnimationHeader);

		std::vector<Animation::Pose> frames;
		frames.resize(header.frameCount);

		for (uint32_t i = 0; i < header.frameCount; i++)
		{
			auto& currFrame = frames[i];
			currFrame.localTRS.resize(header.perFrameTransformCount);

			memcpy_s(currFrame.localTRS.data(), header.perFrameTransformCount * sizeof(Animation::TRS), &totalData[offset], header.perFrameTransformCount * sizeof(Animation::TRS));
			offset += header.perFrameTransformCount * sizeof(Animation::TRS);
		}

		animation->myFrames = frames;
		animation->myDuration = header.duration;
		animation->myFramesPerSecond = header.framesPerSecond;
		return true;
	}

	void AnimationImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<Animation> animation = std::reinterpret_pointer_cast<Animation>(asset);

		std::vector<uint8_t> outData;

		AnimationHeader header{};
		header.perFrameTransformCount = (uint32_t)animation->myFrames.front().localTRS.size();
		header.frameCount = (uint32_t)animation->myFrames.size();
		header.duration = animation->myDuration;
		header.framesPerSecond = animation->myFramesPerSecond;

		outData.resize(sizeof(AnimationHeader) + header.frameCount * header.perFrameTransformCount * sizeof(Animation::TRS));

		size_t offset = 0;
		memcpy_s(&outData[0], sizeof(AnimationHeader), &header, sizeof(AnimationHeader));
		offset += sizeof(AnimationHeader);

		// Copy matrices per frame
		for (const auto& frame : animation->myFrames)
		{
			memcpy_s(&outData[offset], header.perFrameTransformCount * sizeof(Animation::TRS), frame.localTRS.data(), header.perFrameTransformCount * sizeof(Animation::TRS));
			offset += header.perFrameTransformCount * sizeof(Animation::TRS);
		}

		std::ofstream fout(AssetManager::GetContextPath(asset->path) / asset->path, std::ios::binary | std::ios::out);
		fout.write((char*)outData.data(), outData.size());
		fout.close();
	}

	void AnimationImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{
	}

	bool AnimationImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}
}
