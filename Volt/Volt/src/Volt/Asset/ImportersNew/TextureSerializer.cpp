#include "vtpch.h"
#include "TextureSerializer.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	namespace Utility
	{
		inline static bool IsEncodedFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::BC1:
				case ImageFormat::BC1SRGB:
				case ImageFormat::BC2:
				case ImageFormat::BC2SRGB:
				case ImageFormat::BC3:
				case ImageFormat::BC3SRGB:
				case ImageFormat::BC4:
				case ImageFormat::BC5:
				case ImageFormat::BC7:
				case ImageFormat::BC7SRGB:
				case ImageFormat::BC6H_SF16:
				case ImageFormat::BC6H_UF16:
					return true;
			}
			return false;
		}
	}

	struct TextureMip
	{
		uint32_t width;
		uint32_t height;
		size_t dataSize;
	};

	struct TextureHeader
	{
		ImageFormat encoding; // Should be one of the BC formats
		std::vector<TextureMip> mips;
	};

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(asset);
		Ref<Image2D> image = texture->GetImage();

		std::vector<uint8_t> data;

		if (!Utility::IsEncodedFormat(texture->GetImage()->GetFormat()))
		{
			// do encoding
		}
		else
		{
			VkBuffer hostBuffer{};
			VmaAllocation hostAllocation{};

			VulkanAllocator allocator{ "Texture - Serialize" };

			// Create host buffer
			{
				VkBufferCreateInfo info{};
				info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				info.size = texture->GetImage()->GetAllocationSize();
				info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
				info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				hostAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_GPU_TO_CPU, hostBuffer);
			}

			Utility::TransitionImageLayout(image->GetHandle(), image->GetLayout(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			size_t bufferOffset = 0;

			Utility::CopyImageToBuffer(hostBuffer, bufferOffset, image->GetHandle(), image->GetWidth(), image->GetHeight(), 0);
			Utility::TransitionImageLayout(image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->GetLayout());
		
			auto* imageData = allocator.MapMemory<uint8_t>(hostAllocation);
			data.resize(image->GetAllocationSize());

			memcpy_s(data.data(), data.size(), imageData, image->GetAllocationSize());
			allocator.UnmapMemory(hostAllocation);
		
			allocator.DestroyBuffer(hostBuffer, hostAllocation);
		}

		TextureHeader header{};
		header.encoding = image->GetFormat();
	
		{
			auto& firstMip = header.mips.emplace_back();
			firstMip.width = image->GetWidth();
			firstMip.height = image->GetHeight();
		}
	}

	bool TextureSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		return false;
	}
}
