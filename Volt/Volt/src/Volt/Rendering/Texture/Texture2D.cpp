#include "vtpch.h"
#include "Texture2D.h"

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	Texture2D::Texture2D(RHI::Format format, uint32_t width, uint32_t height, const void* data)
	{
		RHI::ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = RHI::ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		m_image = RHI::Image2D::Create(imageSpec, data);
	}

	Texture2D::Texture2D(Ref<RHI::Image2D> image)
		: m_image(image)
	{
	}

	Texture2D::~Texture2D()
	{
		m_image = nullptr;
	}

	const uint32_t Texture2D::GetWidth() const
	{
		return m_image->GetWidth();
	}

	const uint32_t Texture2D::GetHeight() const
	{
		return m_image->GetHeight();
	}

	Ref<Texture2D> Texture2D::Create(RHI::Format format, uint32_t width, uint32_t height, const void* data)
	{
		return CreateRef<Texture2D>(format, width, height, data);
	}

	Ref<Texture2D> Texture2D::Create(Ref<RHI::Image2D> image)
	{
		return CreateRef<Texture2D>(image);
	}
}
