#include "vtpch.h"
#include "DefaultTextureImporter.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"

#include <stb/stb_image.h>

namespace Volt
{
	bool DefaultTextureImporter::ImportTextureImpl(const std::filesystem::path& path, Texture2D& outTexture)
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

		ImageFormat format = ImageFormat::RGBA;

		if (isHDR)
		{
			format = ImageFormat::RGBA32F;
		}

		ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = width;
		imageSpec.height = height;

		Ref<Image2D> image = Image2D::Create(imageSpec, data);
		outTexture.myImage = image;

		stbi_image_free(data);

		return true;
	}
}
