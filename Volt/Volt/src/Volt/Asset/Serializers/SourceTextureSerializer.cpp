#include "vtpch.h"
#include "SourceTextureSerializer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Importers/TextureImporter.h"
#include "Volt/Asset/TextureSource.h"

#include "Volt/Rendering/Texture/Texture2D.h"

namespace Volt
{
	bool SourceTextureSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(LogVerbosity::Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		Ref<Texture2D> tempTex = CreateRef<Texture2D>();


		Ref<TextureSource> destinationTexture = std::reinterpret_pointer_cast<TextureSource>(destinationAsset);
		if (!TextureImporter::ImportTexture(filePath, *tempTex))
		{
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		destinationTexture->m_image = tempTex->GetImage();
		return true;
	}
}
