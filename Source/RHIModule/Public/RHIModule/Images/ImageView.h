#pragma once

#include "RHIModule/Core/RHIInterface.h"
#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	struct ImageViewSpecification
	{
		ImageViewType viewType = ImageViewType::View2D;

		uint32_t baseMipLevel = 0;
		uint32_t baseArrayLayer = 0;
		uint32_t mipCount = 1;
		uint32_t layerCount = 1;

		RHIResource* image = nullptr;
	};

	class VTRHI_API ImageView : public RHIInterface
	{
	public:
		static RefPtr<ImageView> Create(const ImageViewSpecification& specification);

		virtual const ImageAspect GetImageAspect() const = 0;
		virtual const uint64_t GetDeviceAddress() const = 0;
		virtual const ImageUsage GetImageUsage() const = 0;
		virtual const ImageViewType GetViewType() const = 0;
		virtual const bool IsSwapchainView() const = 0;

	protected:
		ImageView() = default;
		~ImageView() override = default;
	};
}
