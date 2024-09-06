#pragma once

#include "Volt/Asset/Importers/TextureImporter.h"

namespace Volt
{
	class DDSTextureImporter : public TextureImporter
	{
	public:
		~DDSTextureImporter() override = default;

	protected:
		bool ImportTextureImpl(const std::filesystem::path& path, Texture2D& outTexture) override;
	};
}
