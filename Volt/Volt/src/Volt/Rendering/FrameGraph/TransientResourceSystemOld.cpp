#include "vtpch.h"
#include "TransientResourceSystemOld.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	inline size_t HashCombine(size_t lhs, size_t rhs)
	{
		return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
	}

	TransientResourceSystemOld::TransientResourceSystemOld()
	{
		//myImagePool = CreateRef<GPUImageMemoryPool>(384 * 1024 * 1024, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	TransientResourceSystemOld::~TransientResourceSystemOld()
	{
		Clear();
	}

	void TransientResourceSystemOld::Clear()
	{
		VT_PROFILE_FUNCTION();

		//auto device = GraphicsContextVolt::GetDevice();
		//for (const auto& [hash, imageDatas] : myCachedImageResources)
		//{
		//	for (const auto& imageData : imageDatas)
		//	{
		//		Renderer::SubmitResourceChange([imageData]()
		//		{
		//			auto device = GraphicsContextVolt::GetDevice();

		//			{
		//				VT_PROFILE_SCOPE("Destroy Image Views");
		//				for (auto& [mip, view] : imageData.image->myImageViews)
		//				{
		//					vkDestroyImageView(device->GetHandle(), view, nullptr);
		//				}

		//				for (auto& [mip, view] : imageData.image->myArrayImageViews)
		//				{
		//					vkDestroyImageView(device->GetHandle(), view, nullptr);
		//				}
		//			}

		//			{
		//				VT_PROFILE_SCOPE("Destroy Image");
		//				vkDestroyImage(device->GetHandle(), imageData.image->myImage, nullptr);
		//			}
		//		});

		//		{
		//			VT_PROFILE_SCOPE("Destroy allocations");
		//			VulkanAllocator allocator{};
		//			allocator.Free(imageData.image->myAllocation);
		//		}
		//	}

		//}

		myCachedImageResources.clear();
	}

	void TransientResourceSystemOld::Reset()
	{
		for (const auto& [index, imageData] : myImageResources)
		{
			const size_t hash = GetHash(imageData.specification);
			myCachedImageResources[hash].emplace_back(imageData);
		}

		myImageResources.clear();
	}

	Weak<Image2D> TransientResourceSystemOld::AquireTexture(const FrameGraphTextureSpecification& textureSpecification, FrameGraphResourceHandle resourceHandle)
	{
		VT_PROFILE_FUNCTION();

		std::scoped_lock lock{ myAquireMutex };

		if (myImageResources.contains(resourceHandle))
		{
			return myImageResources.at(resourceHandle).image;
		}

		const size_t hash = GetHash(textureSpecification);
		if (myCachedImageResources.contains(hash) && !myCachedImageResources.at(hash).empty())
		{
			ImageData image = myCachedImageResources.at(hash).back();
			myCachedImageResources.at(hash).pop_back();

			myImageResources[resourceHandle] = image;
			image.image->SetName(textureSpecification.name);
			return image.image;
		}

		ImageSpecification imageSpecification{};
		imageSpecification.format = textureSpecification.format;
		imageSpecification.usage = textureSpecification.usage;

		imageSpecification.width = textureSpecification.width;
		imageSpecification.height = textureSpecification.height;
		imageSpecification.isCubeMap = textureSpecification.isCubeMap;
		imageSpecification.layers = textureSpecification.layers;
		imageSpecification.debugName = textureSpecification.name;
		imageSpecification.mips = textureSpecification.mips;
		imageSpecification.generateMips = false;

		Ref<Image2D> image = Image2D::Create(imageSpecification, false);
		myImageResources[resourceHandle] = { image, textureSpecification };

		if (imageSpecification.mips > 1)
		{
			image->CreateMipViews();
		}

		return image;
	}

	size_t TransientResourceSystemOld::GetHash(const FrameGraphTextureSpecification& textureSpecification)
	{
		size_t result = std::hash<uint32_t>()((uint32_t)textureSpecification.format);
		result = HashCombine(result, std::hash<uint32_t>()((uint32_t)textureSpecification.usage));
		result = HashCombine(result, std::hash<uint32_t>()((uint32_t)textureSpecification.clearMode));
		result = HashCombine(result, std::hash<uint32_t>()(textureSpecification.width));
		result = HashCombine(result, std::hash<uint32_t>()(textureSpecification.height));
		result = HashCombine(result, std::hash<bool>()(textureSpecification.isCubeMap));
		result = HashCombine(result, std::hash<uint32_t>()(textureSpecification.layers));
		result = HashCombine(result, std::hash<uint32_t>()(textureSpecification.mips));

		return result;
	}
}