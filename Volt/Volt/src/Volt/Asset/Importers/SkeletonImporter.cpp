#include "vtpch.h"
#include "SkeletonImporter.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Log/Log.h"

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool SkeletonImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Skeleton>();
		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(asset);

		const auto filePath = ProjectManager::GetDirectory() / path;

		if (!std::filesystem::exists(filePath)) [[unlikely]]
		{
			VT_CORE_ERROR("File {0} not found!", path.string().c_str());
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		std::ifstream file(filePath);
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

		YAML::Node skeletonNode = root["Skeleton"];
		if (!skeletonNode)
		{
			VT_CORE_ERROR("File {0} is corrupted!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		VT_DESERIALIZE_PROPERTY(name, skeleton->myName, skeletonNode, std::string("Null"));
		
		YAML::Node jointsNode = skeletonNode["joints"];
		if (jointsNode)
		{
			for (const auto& jointNode : jointsNode)
			{
				auto& joint = skeleton->myJoints.emplace_back();
				VT_DESERIALIZE_PROPERTY(parentIndex, joint.parentIndex, jointNode, -1);
				VT_DESERIALIZE_PROPERTY(name, joint.name, jointNode, std::string("Null"));
			}
		}

		YAML::Node invBindPosesNode = skeletonNode["inverseBindPoses"];
		if (invBindPosesNode)
		{
			for (const auto& invBindPoseNode : invBindPosesNode)
			{
				VT_DESERIALIZE_PROPERTY(invBindPose, skeleton->myInverseBindPoses.emplace_back(), invBindPoseNode, gem::mat4(1.f));
			}
		}

		return true;
	}

	void SkeletonImporter::Save(const Ref<Asset>& asset) const
	{
		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Skeleton" << YAML::Value;
		{
			out << YAML::BeginMap;
			VT_SERIALIZE_PROPERTY(name, skeleton->myName, out);

			out << YAML::Key << "joints" << YAML::BeginSeq;
			for (const auto& joint : skeleton->myJoints)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(parentIndex, joint.parentIndex, out);
				VT_SERIALIZE_PROPERTY(name, joint.name, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "inverseBindPoses" << YAML::BeginSeq;
			for (const auto& invBindPose : skeleton->myInverseBindPoses)
			{
				out << YAML::BeginMap;
				VT_SERIALIZE_PROPERTY(invBindPose, invBindPose, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(ProjectManager::GetDirectory() / asset->path);
		fout << out.c_str();
		fout.close();
	}
	void SkeletonImporter::SaveBinary(uint8_t*, const Ref<Asset>&) const
	{}
	bool SkeletonImporter::LoadBinary(const uint8_t*, const AssetPacker::AssetHeader&, Ref<Asset>&) const
	{
		return false;
	}
}