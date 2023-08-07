#pragma once

#include "VoltRHI/Core/RHIInterface.h"
#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	class Image2D;

	struct ImageViewSpecification
	{
		ImageViewType viewType = ImageViewType::View2D;

		uint32_t baseMipLevel = 0;
		uint32_t baseArrayLayer = 0;
		uint32_t mipCount = 1;
		uint32_t layerCount = 1;

		Weak<Image2D> image;
	};

	class ImageView : public RHIInterface
	{
	public:
		static Ref<ImageView> Create(const ImageViewSpecification specification);

	protected:
		ImageView() = default;
		~ImageView() override = default;
	};
}
