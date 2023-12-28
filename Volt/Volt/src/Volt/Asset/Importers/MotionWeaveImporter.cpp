#include "vtpch.h"
#include "MotionWeaveImporter.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/MotionWeaveAsset.h"

#include <yaml-cpp/yaml.h>
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

bool Volt::MotionWeaveImporter::Load(const AssetMetadata& metadata, Ref<Asset>& asset) const
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
	asset = CreateRef<MotionWeaveAsset>();
	auto rAsset = reinterpret_pointer_cast<MotionWeaveAsset>(asset);

	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());
	
	rAsset->m_TargetSkeletonHandle = root["TargetSkeleton"].as<Volt::AssetHandle>(AssetHandle(0));

	if (AssetManager::GetMetadataFromHandle(rAsset->m_TargetSkeletonHandle).IsValid() == false)
	{
		VT_CORE_ERROR("Target skeleton handle is not valid: {0}!", rAsset->m_TargetSkeletonHandle);
		asset->SetFlag(AssetFlag::Invalid, true);
		return false;
	}

	return true;
}

void Volt::MotionWeaveImporter::Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const
{
	Ref<MotionWeaveAsset> motionWeaveAsset = std::reinterpret_pointer_cast<MotionWeaveAsset>(asset);
	YAML::Emitter out;

	out << YAML::BeginMap;
	out << YAML::Key << "TargetSkeleton" << YAML::Value << motionWeaveAsset->m_TargetSkeletonHandle;
	out << YAML::EndMap;
	std::ofstream fout(AssetManager::GetFilesystemPath(metadata.filePath));
	fout << out.c_str();
	fout.close();
}
