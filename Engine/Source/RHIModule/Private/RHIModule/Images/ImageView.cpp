#include "rhipch.h"

#include "RHIModule/Images/ImageView.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<ImageView> ImageView::Create(const ImageViewSpecification& specification)
	{
		return RHIProxy::GetInstance().CreateImageView(specification);
	}
}
