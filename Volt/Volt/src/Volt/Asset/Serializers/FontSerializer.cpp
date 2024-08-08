#include "vtpch.h"
#include "FontSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Text/Font.h"

namespace Volt
{
	void FontSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		VT_ASSERT_MSG(false, "[FontSerializer]: Asset it not serializable");
	}

	bool FontSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Font> font = std::reinterpret_pointer_cast<Font>(destinationAsset);
		font->Initialize(filePath);

		return true;
	}
}
