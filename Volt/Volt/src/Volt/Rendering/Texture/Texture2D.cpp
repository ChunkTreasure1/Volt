#include "vtpch.h"
#include "Texture2D.h"

#include "Volt/Core/Graphics/GraphicsContext.h"

namespace Volt
{
	Texture2D::Texture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
		ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		myImage = Image2D::Create(imageSpec, data);
	}

	Texture2D::Texture2D(Ref<Image2D> image)
		: myImage(image)
	{}

	Texture2D::~Texture2D()
	{
		myImage = nullptr;
	}

	const uint32_t Texture2D::GetWidth() const
	{
		return myImage->GetWidth();
	}

	const uint32_t Texture2D::GetHeight() const
	{
		return myImage->GetHeight();
	}

	void Texture2D::Bind(uint32_t aSlot) const
	{
		auto context = GraphicsContext::GetContext();

		context->PSSetShaderResources(aSlot, 1, myImage->GetSRV().GetAddressOf()); // TODO: How should we handle other shaders?
	}

	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, const void* data)
	{
		return CreateRef<Texture2D>(format, width, height, data);
	}

	Ref<Texture2D> Texture2D::Create(Ref<Image2D> image)
	{
		return CreateRef<Texture2D>(image);
	}
}