#include "vtpch.h"
#include "Texture2D.h"

#include "Volt/Rendering/Resources/GlobalResourceManager.h"

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	Texture2D::Texture2D(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data)
	{
		RHI::ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = RHI::ImageUsage::Texture;
		imageSpec.width = static_cast<uint32_t>(width);
		imageSpec.height = static_cast<uint32_t>(height);

		m_image = RHI::Image2D::Create(imageSpec, data);
		m_resourceHandle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(m_image->GetView());
	}

	Texture2D::Texture2D(Ref<RHI::Image2D> image)
		: m_image(image)
	{
		m_resourceHandle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(m_image->GetView());
	}

	Texture2D::~Texture2D()
	{
		if (m_image)
		{
			GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(m_resourceHandle);
		}
		
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

	ResourceHandle Texture2D::GetResourceHandle() const
	{
		return m_resourceHandle;
	}

	void Texture2D::SetImage(Ref<RHI::Image2D> image)
	{
		if (m_image)
		{
			GlobalResourceManager::UnregisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(m_resourceHandle);
		}

		m_resourceHandle = GlobalResourceManager::RegisterResource<RHI::ImageView, ResourceSpecialization::Texture2D>(image->GetView());

		m_image = image;
	}


	Ref<Texture2D> Texture2D::Create(RHI::PixelFormat format, uint32_t width, uint32_t height, const void* data)
	{
		return CreateRef<Texture2D>(format, width, height, data);
	}

	Ref<Texture2D> Texture2D::Create(Ref<RHI::Image2D> image)
	{
		return CreateRef<Texture2D>(image);
	}
}
