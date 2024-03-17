#include "vtpch.h"
#include "SkeletonImporter.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Project/ProjectManager.h"

#include "Volt/Log/Log.h"

#include <CoreUtilities/FileIO/YAMLStreamReader.h>
#include <CoreUtilities/FileIO/YAMLStreamWriter.h>

#include <yaml-cpp/yaml.h>

namespace Volt
{
	bool SkeletonImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		asset = CreateRef<Skeleton>();
		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(asset);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			asset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		YAMLStreamReader streamReader{};
		if (!streamReader.OpenFile(filePath))
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			asset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		streamReader.EnterScope("Skeleton");

		skeleton->myName = streamReader.ReadAtKey("name", std::string("Null"));

		if (streamReader.HasKey("joints"))
		{
			streamReader.ForEach("joints", [&]() 
			{
				auto& joint = skeleton->myJoints.emplace_back();
				joint.parentIndex = streamReader.ReadAtKey("parentIndex", -1);
				joint.name = streamReader.ReadAtKey("name", std::string("Null"));
			});
		}

		if (streamReader.HasKey("inverseBindPoses"))
		{
			streamReader.ForEach("inverseBindPoses", [&]() 
			{
				skeleton->myInverseBindPose.emplace_back() = streamReader.ReadAtKey("invBindPose", glm::mat4{ 1.f });
			});
		}

		if (streamReader.HasKey("restPose"))
		{
			streamReader.ForEach("restPose", [&]() 
			{
				auto& trs = skeleton->myRestPose.emplace_back();
				trs.position = streamReader.ReadAtKey("position", glm::vec3{ 0.f });
				trs.rotation = streamReader.ReadAtKey("rotation", glm::quat{});
				trs.scale = streamReader.ReadAtKey("scale", glm::vec3{ 1.f });
			});
		}

		streamReader.ExitScope();
		return true;
	}

	void SkeletonImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Skeleton> skeleton = std::reinterpret_pointer_cast<Skeleton>(asset);

		YAMLStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Skeleton");

		streamWriter.SetKey("name", skeleton->myName);

		streamWriter.BeginSequence("joints");
		for (const auto& joint : skeleton->myJoints)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("parentIndex", joint.parentIndex);
			streamWriter.SetKey("name", joint.name);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("inverseBindPose");
		for (const auto& invBindPose : skeleton->myInverseBindPose)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("invBindPose", invBindPose);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("restPose");
		for (const auto& restPose : skeleton->myRestPose)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("position", restPose.position);
			streamWriter.SetKey("rotation", restPose.rotation);
			streamWriter.SetKey("scale", restPose.scale);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.EndMap();
		streamWriter.EndMap();
		streamWriter.WriteToDisk();
	}
}
