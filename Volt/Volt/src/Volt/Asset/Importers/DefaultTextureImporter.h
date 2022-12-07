#pragma once

#include "Volt/Asset/Importers/TextureImporter.h"

namespace Volt
{
	class DefaultTextureImporter : public TextureImporter
	{
	public:
		~DefaultTextureImporter() override = default;

	protected:
		Ref<Texture2D> ImportTextureImpl(const std::filesystem::path& path) override;
	};
}