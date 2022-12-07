#pragma once

#include "Volt/Asset/Importers/TextureImporter.h"

namespace Volt
{
	class DDSTextureImporter : public TextureImporter
	{
	public:
		~DDSTextureImporter() override = default;

	protected:
		Ref<Texture2D> ImportTextureImpl(const std::filesystem::path& path) override;
	};
}