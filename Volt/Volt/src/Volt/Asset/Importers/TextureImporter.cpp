#include "vtpch.h"
#include "TextureImporter.h"

#include "DDSTextureImporter.h"
#include "DefaultTextureImporter.h"

namespace Volt
{
	void TextureImporter::Initialize()
	{
		s_importers[TextureFormat::DDS] = CreateScope<DDSTextureImporter>();
		s_importers[TextureFormat::Other] = CreateScope<DefaultTextureImporter>();
	}

	void TextureImporter::Shutdown()
	{
		s_importers.clear();
	}

	bool TextureImporter::ImportTexture(const std::filesystem::path& path, Texture2D& outTexture)
	{
		return s_importers[FormatFromExtension(path)]->ImportTextureImpl(path, outTexture);
	}

	TextureImporter::TextureFormat TextureImporter::FormatFromExtension(const std::filesystem::path& path)
	{
		auto ext = path.extension().string();

		if (ext == ".dds" || ext == ".DDS")
		{
			return TextureFormat::DDS;
		}

		return TextureFormat::Other;
	}

}
