#pragma once

#include "Volt/Asset/Importers/TextureImporter.h"

namespace Volt
{
	class DefaultTextureImporter : public TextureImporter
	{
	public:
		~DefaultTextureImporter() override = default;

	protected:
		bool ImportTextureImpl(const std::filesystem::path& path, Texture2D& outTexture) override;
	};
}
