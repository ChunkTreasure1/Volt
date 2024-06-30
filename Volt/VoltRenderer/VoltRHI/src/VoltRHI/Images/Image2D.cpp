#include "rhipch.h"
#include "Image2D.h"

#include "VoltRHI/RHIProxy.h"
#include "VoltRHI/Memory/Allocator.h"

namespace Volt::RHI
{
	RefPtr<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
	{
		return RHIProxy::GetInstance().CreateImage2D(specification, data, allocator);
	}

	RefPtr<Image2D> Image2D::Create(const SwapchainImageSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateImage2D(specification);
	}
}
