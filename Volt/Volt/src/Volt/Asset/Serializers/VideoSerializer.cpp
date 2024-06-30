#include "vtpch.h"
#include "VideoSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Video/Video.h"

namespace Volt
{
	void VideoSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{

	}

	bool VideoSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Video> video = std::reinterpret_pointer_cast<Video>(destinationAsset);
		video->Initialize(filePath);

		return true;
	}
}
