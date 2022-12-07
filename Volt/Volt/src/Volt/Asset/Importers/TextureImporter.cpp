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

	Ref<Texture2D> TextureImporter::ImportTexture(const std::filesystem::path& path)
	{
		return s_importers[FormatFromExtension(path)]->ImportTextureImpl(path);
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