#include "rhipch.h"

#include "RHIModule/Images/Image.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Image> Image::Create(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
	{
		return RHIProxy::GetInstance().CreateImage(specification, data, allocator);
	}
	RefPtr<Image> Image::Create(const SwapchainImageSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateImage(specification);
	}
}
