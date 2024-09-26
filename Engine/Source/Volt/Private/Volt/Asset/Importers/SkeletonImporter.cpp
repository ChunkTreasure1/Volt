#include "vtpch.h"
#include "Volt/Asset/Importers/SkeletonImporter.h"

#include "Volt/Asset/Animation/Skeleton.h"
#include <AssetSystem/AssetManager.h>

#include "Volt/Utility/YAMLSerializationHelpers.h"
#include "Volt/Project/ProjectManager.h"

#include <CoreUtilities/FileIO/YAMLFileStreamReader.h>
#include <CoreUtilities/FileIO/YAMLFileStreamWriter.h>

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

		streamReader.EnterScope("Skeleton");

		skeleton->m_name = streamReader.ReadAtKey("name", std::string("Null"));

		if (streamReader.HasKey("joints"))
		{
			streamReader.ForEach("joints", [&]()
			{
				auto& joint = skeleton->m_joints.emplace_back();
				joint.parentIndex = streamReader.ReadAtKey("parentIndex", -1);
				joint.name = streamReader.ReadAtKey("name", std::string("Null"));
			});
		}
		
		if (streamReader.HasKey("jointAttachments"))
		{
			streamReader.ForEach("jointAttachments", [&]() 
			{
				auto& attachment = skeleton->m_jointAttachments.emplace_back();
				attachment.name = streamReader.ReadAtKey("name", std::string(""));
				attachment.jointIndex = streamReader.ReadAtKey("jointIndex", int32_t(-1));
				attachment.id = streamReader.ReadAtKey("id", UUID64(0));
				attachment.positionOffset = streamReader.ReadAtKey("positionOffset", glm::vec3{ 0.f });
				attachment.rotationOffset = streamReader.ReadAtKey("rotationOffset", glm::quat{ 1.f, 0.f, 0.f, 0.f });
			});
		}

		if (streamReader.HasKey("inverseBindPoses"))
		{
			streamReader.ForEach("inverseBindPoses", [&]()
			{
				skeleton->m_inverseBindPose.emplace_back() = streamReader.ReadAtKey("invBindPose", glm::mat4{ 1.f });
			});
		}

		if (streamReader.HasKey("restPose"))
		{
			streamReader.ForEach("restPose", [&]()
			{
				auto& trs = skeleton->m_restPose.emplace_back();
				trs.translation = streamReader.ReadAtKey("position", glm::vec3{ 0.f });
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

		YAMLFileStreamWriter streamWriter{ AssetManager::GetFilesystemPath(metadata.filePath) };

		streamWriter.BeginMap();
		streamWriter.BeginMapNamned("Skeleton");

		streamWriter.SetKey("name", skeleton->m_name);

		streamWriter.BeginSequence("joints");
		for (const auto& joint : skeleton->m_joints)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("parentIndex", joint.parentIndex);
			streamWriter.SetKey("name", joint.name);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("jointAttachments");
		for (const auto& attachment : skeleton->m_jointAttachments)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("name", attachment.name);
			streamWriter.SetKey("jointIndex", attachment.jointIndex);
			streamWriter.SetKey("id", attachment.id);
			streamWriter.SetKey("positionOffset", attachment.positionOffset);
			streamWriter.SetKey("rotationOffset", attachment.rotationOffset);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("inverseBindPose");
		for (const auto& invBindPose : skeleton->m_inverseBindPose)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("invBindPose", invBindPose);
			streamWriter.EndMap();
		}
		streamWriter.EndSequence();

		streamWriter.BeginSequence("restPose");
		for (const auto& restPose : skeleton->m_restPose)
		{
			streamWriter.BeginMap();
			streamWriter.SetKey("position", restPose.translation);
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
