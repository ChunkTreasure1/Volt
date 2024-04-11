#include "rhipch.h"
#include "ImageView.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<ImageView> ImageView::Create(const ImageViewSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateImageView(specification);
	}
}
