#include "vtpch.h"
#include "AnimatedCharacterImporter.h"

#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool AnimatedCharacterImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<AnimatedCharacter>();
		Ref<AnimatedCharacter> character = std::reinterpret_pointer_cast<AnimatedCharacter>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLFileStreamReader streamReader{};

		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		AssetHandle skeletonHandle = streamReader.ReadAtKey("skeleton", AssetHandle(0));
		AssetHandle skinHandle = streamReader.ReadAtKey("skin", AssetHandle(0));

		character->mySkeleton = AssetManager::GetAsset<Skeleton>(skeletonHandle);
		character->mySkin = AssetManager::GetAsset<Mesh>(skinHandle);

		if (streamReader.HasKey("animations"))
		{
			streamReader.ForEach("animations", [&]() 
			{
				uint32_t index = streamReader.ReadAtKey("index", uint32_t(-1));
				AssetHandle animHandle = streamReader.ReadAtKey("animation", AssetHandle(0));

				streamReader.ForEach("animationEvents", [&]() 
				{
					auto& newEvent = character->myAnimationEvents[index].emplace_back();
					newEvent.name = streamReader.ReadAtKey("Name", std::string());
					newEvent.frame = streamReader.ReadAtKey("Frame", 0);
				});

				if (index != (uint32_t)-1 && animHandle != Asset::Null())
				{
					character->myAnimations[index] = AssetManager::GetAsset<Animation>(animHandle);
				}
			});
		}

		if (streamReader.HasKey("jointAttachments"))
		{
			streamReader.ForEach("jointAttachments", [&]() 
			{
				auto& newAttachment = character->myJointAttachments.emplace_back();
				newAttachment.name = streamReader.ReadAtKey("name", std::string());
				newAttachment.jointIndex = streamReader.ReadAtKey("jointIndex", -1);
				newAttachment.id = streamReader.ReadAtKey("id", UUID64(0));
				newAttachment.positionOffset = streamReader.ReadAtKey("positionOffset", glm::vec3{ 0.f });
				newAttachment.rotationOffset = streamReader.ReadAtKey("rotationOffset", glm::quat{ 1.f, 0.f, 0.f, 0.f });
			});
		}

		return true;
	}

	void AnimatedCharacterImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<AnimatedCharacter> character = std::reinterpret_pointer_cast<AnimatedCharacter>(asset);

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };
		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("AnimatedCharacter");

		streamWriter.SetKey("skeleton", (character->mySkeleton ? character->mySkeleton->handle : Asset::Null()));
		streamWriter.SetKey("skin", (character->mySkin ? character->mySkin->handle : Asset::Null()));

		streamWriter.BeginSequence("animations");
		for (const auto& [index, anim] : character->myAnimations)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("index", index);
			streamWriter.SetKey("animation", (anim ? anim->handle : Asset::Null()));

			if (character->myAnimationEvents.contains(index))
			{
				streamWriter.BeginSequence("animationEvents");
				for (const auto& event : character->myAnimationEvents.at(index))
				{
					streamWriter.BeginMap();
					streamWriter.SetKey("Name", event.name);
					streamWriter.SetKey("Frame", event.frame);
					streamWriter.EndMap();
				}
				streamWriter.EndSequence();
			}
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("jointAttachments");
		for (const auto& jntAttachment : character->myJointAttachments)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("name", jntAttachment.name);
			streamWriter.SetKey("jointIndex", jntAttachment.jointIndex);
			streamWriter.SetKey("id", jntAttachment.id);
			streamWriter.SetKey("positionOffset", jntAttachment.positionOffset);
			streamWriter.SetKey("rotationOffset", jntAttachment.rotationOffset);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();

		streamWriter.WriteToDisk();
	}
}
