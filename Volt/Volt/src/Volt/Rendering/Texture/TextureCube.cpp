#include "vtpch.h"
#include "TextureCube.h"

namespace Volt
{
	TextureCube::TextureCube(ImageFormat aFormat, uint32_t aWidth, uint32_t aHeight, const void* aData)
	{
		ImageSpecification imageSpec{};
		imageSpec.format = aFormat;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = aWidth;
		imageSpec.height = aHeight;
		imageSpec.layers = 6;
		imageSpec.isCubeMap = true;
	
		myImage = Image2D::Create(imageSpec, aData);
	}

	TextureCube::~TextureCube()
	{
		myImage = nullptr;
	}

	const uint32_t TextureCube::GetWidth() const
	{
		return myImage->GetWidth();
	}

	const uint32_t TextureCube::GetHeight() const
	{
		return myImage->GetHeight();
	}

	Ref<TextureCube> TextureCube::Create(ImageFormat aFormat, uint32_t aWidth, uint32_t aHeight, const void* aData)
	{
		return CreateRef<TextureCube>(aFormat, aWidth, aHeight, aData);
	}
}