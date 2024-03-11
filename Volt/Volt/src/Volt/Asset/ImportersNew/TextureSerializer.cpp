#include "vtpch.h"
#include "TextureSerializer.h"

#include "Volt/Asset/AssetManager.h"

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
		size_t dataOffset;
	};

	struct TextureHeader
	{
		ImageFormat format; // Should be one of the BC formats
		std::vector<TextureMip> mips;

		static void Serialize(BinaryStreamWriter& streamWriter, const TextureHeader& data)
		{
			streamWriter.Write(data.format);
			streamWriter.Write(data.mips);
		}

		static void Deserialize(BinaryStreamReader& streamReader, TextureHeader& outData)
		{
			streamReader.Read(outData.format);
			streamReader.Read(outData.mips);
		}
	};

	struct TextureData
	{
		struct Mip 
		{
			uint32_t width;
			uint32_t height;
			size_t dataSize;
			size_t dataOffset;
			const void* dataPtr = nullptr;
		};

		void SetupMips(const TextureHeader& header, const Buffer& buffer)
		{
			for (const auto& mip : header.mips)
			{
				auto& newMip = mips.emplace_back();
				newMip.width = mip.width;
				newMip.height = mip.height;
				newMip.dataSize = mip.dataSize;
				newMip.dataOffset = mip.dataOffset;
				newMip.dataPtr = buffer.As<const void>(newMip.dataOffset);
			}
		}

		std::vector<Mip> mips;
	};

	constexpr uint32_t CURRENT_ASSET_VERSION = 1;

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(asset);
		Ref<Image2D> image = texture->GetImage();

		TextureHeader header{};
		header.format = image->GetFormat();

		Buffer dataBuffer{};

		if (!Utility::IsEncodedFormat(texture->GetImage()->GetFormat()))
		{
			// do encoding
		}
		else
		{
			for (uint32_t i = 0; i < image->GetSpecification().mips; i++)
			{
				auto& newMip = header.mips.emplace_back();
				newMip.width = image->GetWidth() >> i;
				newMip.height = image->GetHeight() >> i;
				newMip.dataOffset = dataBuffer.GetSize();

				newMip.dataSize = image->CopyToBuffer(dataBuffer, i, dataBuffer.GetSize());
			}
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, CURRENT_ASSET_VERSION, streamWriter);

		streamWriter.Write(header);
		streamWriter.Write(dataBuffer);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool TextureSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_CORE_ERROR("File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_CORE_ERROR("Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);

		TextureHeader textureHeader{};
		streamReader.Read(textureHeader);
		
		Buffer textureDataBuffer{};
		streamReader.Read(textureDataBuffer);

		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(destinationAsset);

		// Create image
		{
			VkBuffer stagingBuffer;
			VmaAllocation stagingAllocation;

			VulkanAllocator allocator{};

			const size_t size = textureHeader.mips.front().dataSize;

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
				memcpy_s(bufferPtr, size, textureDataBuffer.As<void>(), size);
				allocator.UnmapMemory(stagingAllocation);
			}

			ImageSpecification specification{};
			specification.format = textureHeader.format;
			specification.usage = ImageUsage::Texture;
			specification.width = textureHeader.mips.front().width;
			specification.height = textureHeader.mips.front().height;
			specification.mips = static_cast<uint32_t>(textureHeader.mips.size());
			specification.generateMips = false;
			specification.debugName = filePath.stem().string();

			Ref<Image2D> image = Image2D::Create(specification);

			TextureData texData{};
			texData.SetupMips(textureHeader, textureDataBuffer);

			Utility::TransitionImageLayout(image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Utility::CopyBufferToImage(stagingBuffer, image->GetHandle(), specification.width, specification.height);

			allocator.DestroyBuffer(stagingBuffer, stagingAllocation);

			for (size_t i = 1; i < texData.mips.size(); i++)
			{
				VmaAllocation mipStagingAllocation;
				VkBuffer mipStagingBuffer;

				auto mipData = texData.mips.at(i);
				const VkDeviceSize mipSize = mipData.dataSize;

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
					memcpy_s(bufferPtr, mipSize, mipData.dataPtr, mipSize);
					allocator.UnmapMemory(mipStagingAllocation);
				}

				Utility::CopyBufferToImage(mipStagingBuffer, image->GetHandle(), mipData.width, mipData.height, static_cast<uint32_t>(i));
				allocator.DestroyBuffer(mipStagingBuffer, mipStagingAllocation);
			}

			Utility::TransitionImageLayout(image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			image->OverrideLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			textureDataBuffer.Release();
			texture->myImage = image;
		}

		return true;
	}
}
