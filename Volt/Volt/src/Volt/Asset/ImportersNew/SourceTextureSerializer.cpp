#include "vtpch.h"
#include "SourceTextureSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Importers/TextureImporter.h"

namespace Volt
{
	bool SourceTextureSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Texture2D> destinationTexture = std::reinterpret_pointer_cast<Texture2D>(destinationAsset);
		if (!TextureImporter::ImportTexture(filePath, *destinationTexture))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		return true;
	}
}
