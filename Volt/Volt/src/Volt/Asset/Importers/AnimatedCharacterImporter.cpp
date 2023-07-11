#include "vtpch.h"
#include "AnimatedCharacterImporter.h"

#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Utility/SerializationMacros.h"

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

		std::ifstream file(filePath);
		if (!file.is_open())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		std::stringstream sstream;
		sstream << file.rdbuf();
		file.close();

		YAML::Node root;

		try
		{
			root = YAML::Load(sstream.str());
		}
		catch (std::exception& e)
		{
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", metadata.filePath, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node characterNode = root["AnimatedCharacter"];
		if (!characterNode)
		{
			VT_CORE_ERROR("File {0} is corrupted!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		AssetHandle skeletonHandle;
		VT_DESERIALIZE_PROPERTY(skeleton, skeletonHandle, characterNode, AssetHandle(0));

		AssetHandle skinHandle;
		VT_DESERIALIZE_PROPERTY(skin, skinHandle, characterNode, AssetHandle(0));

		character->mySkeleton = AssetManager::GetAsset<Skeleton>(skeletonHandle);
		character->mySkin = AssetManager::GetAsset<Mesh>(skinHandle);

		YAML::Node animationsNode = characterNode["animations"];
		if (animationsNode)
		{
			for (const auto& animNode : animationsNode)
			{
				uint32_t index;
				VT_DESERIALIZE_PROPERTY(index, index, animNode, (uint32_t)-1);

				AssetHandle animHandle;
				VT_DESERIALIZE_PROPERTY(animation, animHandle, animNode, AssetHandle(0));

				for (const auto& event : animNode["animationEvents"])
				{
					auto& newEvent = character->myAnimationEvents[index].emplace_back();
					VT_DESERIALIZE_PROPERTY(Name, newEvent.name, event, std::string(""));
					VT_DESERIALIZE_PROPERTY(Frame, newEvent.frame, event, 0);
				}

				if (index != (uint32_t)-1 && animHandle != Asset::Null())
				{
					character->myAnimations[index] = AssetManager::GetAsset<Animation>(animHandle);
				}
			}
		}

		YAML::Node jointAttachmentNode = characterNode["jointAttachments"];
		if (jointAttachmentNode)
		{
			for (const auto& attachmentNode : jointAttachmentNode)
			{
				auto& newAttachment = character->myJointAttachments.emplace_back();
				VT_DESERIALIZE_PROPERTY(name, newAttachment.name, attachmentNode, std::string("Empty"));
				VT_DESERIALIZE_PROPERTY(jointIndex, newAttachment.jointIndex, attachmentNode, -1);
				VT_DESERIALIZE_PROPERTY(id, newAttachment.id, attachmentNode, UUID(0));
				VT_DESERIALIZE_PROPERTY(positionOffset, newAttachment.positionOffset, attachmentNode, glm::vec3{ 0.f });
				VT_DESERIALIZE_PROPERTY(rotationOffset, newAttachment.rotationOffset, attachmentNode, glm::quat(1.f, 0.f, 0.f, 0.f));
			}
		}

		return true;
	}

	void AnimatedCharacterImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<AnimatedCharacter> character = std::reinterpret_pointer_cast<AnimatedCharacter>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "AnimatedCharacter" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(skeleton, (character->mySkeleton ? character->mySkeleton->handle : Asset::Null()), out);
			VT_SERIALIZE_PROPERTY(skin, (character->mySkin ? character->mySkin->handle : Asset::Null()), out);

			out << YAML::Key << "animations" << YAML::BeginSeq;
			for (const auto& [index, anim] : character->myAnimations)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(index, index, out);
				VT_SERIALIZE_PROPERTY(animation, (anim ? anim->handle : Asset::Null()), out);

				if (character->myAnimationEvents.contains(index))
				{
					out << YAML::Key << "animationEvents" << YAML::BeginSeq;
					for (const auto& event : character->myAnimationEvents.at(index))
					{
						out << YAML::BeginMap;
						VT_SERIALIZE_PROPERTY(Name, event.name, out);
						VT_SERIALIZE_PROPERTY(Frame, event.frame, out);
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "jointAttachments" << YAML::BeginSeq;
			for (const auto& jntAttachment : character->myJointAttachments)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(name, jntAttachment.name, out);
				VT_SERIALIZE_PROPERTY(jointIndex, jntAttachment.jointIndex, out);
				VT_SERIALIZE_PROPERTY(id, jntAttachment.id, out);
				VT_SERIALIZE_PROPERTY(positionOffset, jntAttachment.positionOffset, out);
				VT_SERIALIZE_PROPERTY(rotationOffset, jntAttachment.rotationOffset, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
		fout << out.c_str();
		fout.close();
	}
}
