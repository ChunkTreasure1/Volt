#include "vtpch.h"
#include "DDSTextureImporter.h"

#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Utility/ImageUtility.h"

#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>

namespace tdl = tinyddsloader;

namespace Volt
{
	namespace Utility
	{
		static void PrintDDSError(tdl::Result code, const std::filesystem::path& path)
		{
			switch (code)
			{
				case tdl::ErrorFileOpen: VT_CORE_ERROR("Unable to open texture {0}!", path.string().c_str()); break;
				case tdl::ErrorRead: VT_CORE_ERROR("Unable to read texture {0}!", path.string().c_str()); break;
				case tdl::ErrorMagicWord: VT_CORE_ERROR("Unable to read magic word in texture {0}!", path.string().c_str()); break;
				case tdl::ErrorSize: VT_CORE_ERROR("Size is wrong in texture {0}!", path.string().c_str()); break;
				case tdl::ErrorVerify: VT_CORE_ERROR("Unable to verify texture {0}!", path.string().c_str()); break;
				case tdl::ErrorNotSupported: VT_CORE_ERROR("Texture type of texture {0} not supported!", path.string().c_str()); break;
				case tdl::ErrorInvalidData: VT_CORE_ERROR("Invalid data in texture {0}!", path.string().c_str()); break;
			}
		}

		static ImageFormat DDSToLampImageFormat(tdl::DDSFile::DXGIFormat format)
		{
			switch (format)
			{
				case tdl::DDSFile::DXGIFormat::R32G32B32A32_Float: return ImageFormat::RGBA32F;
				case tdl::DDSFile::DXGIFormat::R16G16B16A16_Float: return ImageFormat::RGBA16F;

				case tdl::DDSFile::DXGIFormat::R32G32_Float: return ImageFormat::RG32F;
				case tdl::DDSFile::DXGIFormat::R16G16_Float: return ImageFormat::RG16F;

				case tdl::DDSFile::DXGIFormat::R8G8B8A8_UNorm: return ImageFormat::RGBA;
				case tdl::DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB: return ImageFormat::SRGB;

				case tdl::DDSFile::DXGIFormat::R32_Float: return ImageFormat::R32F;
				case tdl::DDSFile::DXGIFormat::R32_UInt: return ImageFormat::R32UI;
				case tdl::DDSFile::DXGIFormat::R32_SInt: return ImageFormat::R32SI;

				case tdl::DDSFile::DXGIFormat::BC1_UNorm: return ImageFormat::BC1;
				case tdl::DDSFile::DXGIFormat::BC1_UNorm_SRGB: return ImageFormat::BC1SRGB;

				case tdl::DDSFile::DXGIFormat::BC2_UNorm: return ImageFormat::BC2;
				case tdl::DDSFile::DXGIFormat::BC2_UNorm_SRGB: return ImageFormat::BC2SRGB;

				case tdl::DDSFile::DXGIFormat::BC3_UNorm: return ImageFormat::BC3;
				case tdl::DDSFile::DXGIFormat::BC3_UNorm_SRGB: return ImageFormat::BC3SRGB;

				case tdl::DDSFile::DXGIFormat::BC4_UNorm: return ImageFormat::BC4;

				case tdl::DDSFile::DXGIFormat::BC5_UNorm: return ImageFormat::BC5;

				case tdl::DDSFile::DXGIFormat::BC7_UNorm: return ImageFormat::BC7;
				case tdl::DDSFile::DXGIFormat::BC7_UNorm_SRGB: return ImageFormat::BC7SRGB;
			}

			return ImageFormat::RGBA;
		}
	}

	Ref<Texture2D> DDSTextureImporter::ImportTextureImpl(const std::filesystem::path& path)
	{
		tdl::DDSFile dds;
		auto returnCode = dds.Load(path.string().c_str());
		if (returnCode != tdl::Result::Success)
		{
			Utility::PrintDDSError(returnCode, path);
		}

		if (dds.GetTextureDimension() != tdl::DDSFile::TextureDimension::Texture2D)
		{
			VT_CORE_ERROR("Texture {0} is not 2D!", path.string().c_str());
			return nullptr;
		}

		auto imageData = dds.GetImageData();

		const VkDeviceSize size = imageData->m_memSlicePitch;
		const void* dataPtr = imageData->m_mem;

		const uint32_t width = imageData->m_width;
		const uint32_t height = imageData->m_height;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		VulkanAllocatorVolt allocator{ "Texture2D - Create" };

		// Create staging buffer
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = size;
			info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			stagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		}

		// Map memory
		{
			void* bufferPtr = allocator.MapMemory<void>(stagingAllocation);
			memcpy_s(bufferPtr, size, dataPtr, size);
			allocator.UnmapMemory(stagingAllocation);
		}

		Ref<Image2D> image;

		// Create image
		{
			ImageSpecification specification{};
			specification.format = Utility::DDSToLampImageFormat(dds.GetFormat());
			specification.usage = ImageUsage::Texture;
			specification.width = width;
			specification.height = height;
			specification.mips = dds.GetMipCount();
			specification.generateMips = false;
			specification.debugName = path.stem().string();

			image = Image2D::Create(specification);

			Utility::TransitionImageLayout(image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Utility::CopyBufferToImage(stagingBuffer, image->GetHandle(), width, height);
		}

		allocator.DestroyBuffer(stagingBuffer, stagingAllocation);

		// Set mip data
		{
			for (uint32_t i = 1; i < dds.GetMipCount(); i++)
			{
				VmaAllocation mipStagingAllocation;
				VkBuffer mipStagingBuffer;

				auto mipData = dds.GetImageData(i);
				const VkDeviceSize mipSize = mipData->m_memSlicePitch;

				// Create mip staging buffer
				{
					VkBufferCreateInfo info{};
					info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
					info.size = mipSize;
					info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
					info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

					mipStagingAllocation = allocator.AllocateBuffer(info, VMA_MEMORY_USAGE_CPU_TO_GPU, mipStagingBuffer);
				}

				// Map mip memory
				{
					void* bufferPtr = allocator.MapMemory<void>(mipStagingAllocation);
					memcpy_s(bufferPtr, mipSize, mipData->m_mem, mipSize);
					allocator.UnmapMemory(mipStagingAllocation);
				}

				Utility::CopyBufferToImage(mipStagingBuffer, image->GetHandle(), mipData->m_width, mipData->m_height, i);
				allocator.DestroyBuffer(mipStagingBuffer, mipStagingAllocation);
			}
		}

		//Utility::TransitionImageFromTransferQueue(image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		Utility::TransitionImageLayout(image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		image->OverrideLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Ref<Texture2D> texture = CreateRef<Texture2D>();
		texture->myImage = image;

		return texture;
	}
}
