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
	bool AnimatedCharacterImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<AnimatedCharacter>();
		Ref<AnimatedCharacter> character = std::reinterpret_pointer_cast<AnimatedCharacter>(asset);

		if (!std::filesystem::exists(path)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(path);
		if (!file.is_open()) [[unlikely]]
		{
			VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
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
			VT_CORE_ERROR("{0} contains invalid YAML! Please correct it! Error: {1}", path, e.what());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		YAML::Node characterNode = root["AnimatedCharacter"];
		if (!characterNode)
		{
			VT_CORE_ERROR("File {0} is corrupted!", path.string().c_str());
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

				if (index != (uint32_t)-1 && animHandle != Asset::Null())
				{
					character->myAnimations[index] = AssetManager::GetAsset<Animation>(animHandle);
				}
			}
		}

		return true;
	}

	void AnimatedCharacterImporter::Save(const Ref<Asset>& asset) const
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
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(asset->path);
		fout << out.c_str();
		fout.close();
	}
	void AnimatedCharacterImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}
	bool AnimatedCharacterImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}
}