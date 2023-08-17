#include "vtpch.h"
#include "DefaultTextureImporter.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <stb/stb_image.h>

namespace Volt
{
	Ref<Texture2D> DefaultTextureImporter::ImportTextureImpl(const std::filesystem::path& path)
	{
		int32_t width;
		int32_t height;
		int32_t channels;

		void* data = nullptr;
		bool isHDR = false;

		stbi_set_flip_vertically_on_load(0);
		if (stbi_is_hdr(path.string().c_str()))
		{
			data = stbi_loadf(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
			isHDR = true;
		}
		else
		{
			data = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		}

		RHI::Format format = RHI::Format::R8G8B8A8_UNORM;

		if (isHDR)
		{
			format = RHI::Format::R32G32B32A32_SFLOAT;
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(format, width, height, data);
		stbi_image_free(data);

		return texture;
	}
}
