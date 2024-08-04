#include "vtpch.h"
#include "DefaultTextureImporter.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <RHIModule/Images/Image2D.h>

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
		bool is16Bit = false;

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

		is16Bit = stbi_is_16_bit(path.string().c_str());

		RHI::PixelFormat format = RHI::PixelFormat::R8G8B8A8_UNORM;

		if (isHDR)
		{
			if (is16Bit)
			{
				format = RHI::PixelFormat::R16G16B16A16_SFLOAT;
			}
			else
			{
				format = RHI::PixelFormat::R32G32B32A32_SFLOAT;
			}
		}

		RHI::ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = RHI::ImageUsage::Texture;
		imageSpec.width = width;
		imageSpec.height = height;

		RefPtr<RHI::Image2D> image = RHI::Image2D::Create(imageSpec, data);
		outTexture.SetImage(image);

		stbi_image_free(data);

		return true;
	}


}
