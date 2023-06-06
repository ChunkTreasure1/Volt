#include "vtpch.h"
#include "TransientResourceSystem.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Memory/GPUImageMemoryPool.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Core/Profiling.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	inline size_t HashCombine(size_t lhs, size_t rhs)
	{
		return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
	}

	TransientResourceSystem::TransientResourceSystem()
	{
		myImagePool = CreateRef<GPUImageMemoryPool>(384 * 1024 * 1024, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	TransientResourceSystem::~TransientResourceSystem()
	{
		Clear();
	}

	void TransientResourceSystem::Clear()
	{
		VT_PROFILE_FUNCTION();

		auto device = GraphicsContextVolt::GetDevice();
		for (const auto& [hash, imageDatas] : myCachedImageResources)
		{
			for (const auto& imageData : imageDatas)
			{
				Renderer::SubmitResourceChange([imageData]()
				{
					auto device = GraphicsContextVolt::GetDevice();

					{
						VT_PROFILE_SCOPE("Destroy Image Views");
						for (auto& [mip, view] : imageData.image->myImageViews)
						{
							vkDestroyImageView(device->GetHandle(), view, nullptr);
						}

						for (auto& [mip, view] : imageData.image->myArrayImageViews)
						{
							vkDestroyImageView(device->GetHandle(), view, nullptr);
						}
					}

					{
						VT_PROFILE_SCOPE("Destroy Image");
						vkDestroyImage(device->GetHandle(), imageData.image->myImage, nullptr);
					}
				});

				{
					VT_PROFILE_SCOPE("Destroy allocations");
					VulkanAllocator allocator{};
					allocator.Free(imageData.image->myAllocation);
				}
			}

		}

		myCachedImageResources.clear();
	}

	void TransientResourceSystem::Reset()
	{
		for (const auto& [index, imageData] : myImageResources)
		{
			const size_t hash = GetHash(imageData.specification);
			myCachedImageResources[hash].emplace_back(imageData);
		}

		myImageResources.clear();
	}

	Weak<Image2D> TransientResourceSystem::AquireTexture(const FrameGraphTextureSpecification& textureSpecification, FrameGraphResourceHandle resourceHandle)
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

		Ref<Image2D> image = Image2D::Create(imageSpecification, myImagePool->GetPool());
		myImageResources[resourceHandle] = { image, textureSpecification };

		if (imageSpecification.mips > 1)
		{
			image->CreateMipViews();
		}

		return image;
	}

	size_t TransientResourceSystem::GetHash(const FrameGraphTextureSpecification& textureSpecification)
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
