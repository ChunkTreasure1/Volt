#include "rhipch.h"
#include "Image2D.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data /* = nullptr */)
	{
		return RHIProxy::GetInstance().CreateImage2D(specification, data);
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data)
	{
		return RHIProxy::GetInstance().CreateImage2D(specification, customAllocator, data);
	}

	Ref<Image2D> Image2D::Create(const SwapchainImageSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateImage2D(specification);
	}
}
