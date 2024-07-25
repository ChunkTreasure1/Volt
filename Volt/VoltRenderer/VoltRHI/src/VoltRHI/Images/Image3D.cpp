#include "rhipch.h"
#include "Image3D.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Image3D> Image3D::Create(const ImageSpecification& specification, const void* data, RefPtr<Allocator> allocator)
	{
		return RHIProxy::GetInstance().CreateImage3D(specification, data, allocator);
	}
}
