#include "vtpch.h"
#include "TextureSerializer.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/ImageUtility.h>
#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Memory/Allocation.h>

namespace Volt
{
	struct TextureMip
	{
		uint32_t width;
		uint32_t height;
		size_t dataSize;
		size_t dataOffset;
	};

	struct TextureHeader
	{
		RHI::PixelFormat format; // Should be one of the BC formats
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

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(asset);
		Ref<RHI::Image2D> image = texture->GetImage();

		TextureHeader header{};
		header.format = image->GetFormat();

		Buffer dataBuffer{};

		//if (!Utility::IsEncodedFormat(texture->GetImage()->GetFormat()))
		//{
		//	// do encoding
		//}
		//else
		{
			const size_t maxSize = image->GetWidth() * image->GetHeight() * RHI::Utility::GetByteSizePerPixelFromFormat(image->GetFormat());

			Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();
			Ref<RHI::Allocation> stagingBuffer = RHI::GraphicsContext::GetDefaultAllocator().CreateBuffer(maxSize, RHI::BufferUsage::TransferDst, RHI::MemoryUsage::GPUToCPU);

			commandBuffer->Begin();

			{
				RHI::ResourceBarrierInfo barrier{};
				barrier.type = RHI::BarrierType::Image;
				barrier.imageBarrier().srcStage = RHI::BarrierStage::None;
				barrier.imageBarrier().srcAccess = RHI::BarrierAccess::None;
				barrier.imageBarrier().srcLayout = RHI::ImageLayout::ShaderRead;
				barrier.imageBarrier().dstStage = RHI::BarrierStage::Copy;
				barrier.imageBarrier().dstAccess = RHI::BarrierAccess::TransferSource;
				barrier.imageBarrier().dstLayout = RHI::ImageLayout::TransferSource;
				barrier.imageBarrier().resource = image;

				commandBuffer->ResourceBarrier({ barrier });
			}

			commandBuffer->End();
			commandBuffer->ExecuteAndWait();

			for (uint32_t i = 0; i < image->GetMipCount(); i++)
			{
				auto& newMip = header.mips.emplace_back();
				newMip.width = std::max(image->GetWidth() >> i, 1u);
				newMip.height = std::max(image->GetHeight() >> i, 1u);
				newMip.dataOffset = dataBuffer.GetSize();

				uint32_t sizeWidth = newMip.width;
				uint32_t sizeHeight = newMip.height;

				if (RHI::Utility::IsCompressedFormat(image->GetFormat()))
				{
					sizeWidth = RHI::Utility::NextPow2(sizeWidth);
					sizeHeight = RHI::Utility::NextPow2(sizeHeight);
				}

				const size_t mipSize = sizeWidth * sizeHeight * RHI::Utility::GetByteSizePerPixelFromFormat(image->GetFormat());

				commandBuffer->Begin();
				commandBuffer->CopyImageToBuffer(image, stagingBuffer, 0, newMip.width, newMip.height, i);
				commandBuffer->End();
				commandBuffer->ExecuteAndWait();

				void* data = stagingBuffer->Map<void>();
				dataBuffer.Resize(newMip.dataOffset + mipSize);
				dataBuffer.Copy(data, mipSize, newMip.dataOffset);
				stagingBuffer->Unmap();
			}

			commandBuffer->Begin();

			{
				RHI::ResourceBarrierInfo barrier{};
				barrier.type = RHI::BarrierType::Image;
				barrier.imageBarrier().srcAccess = RHI::BarrierAccess::TransferSource;
				barrier.imageBarrier().srcLayout = RHI::ImageLayout::TransferSource;
				barrier.imageBarrier().srcStage = RHI::BarrierStage::Copy;
				barrier.imageBarrier().dstStage = RHI::BarrierStage::None;
				barrier.imageBarrier().dstAccess = RHI::BarrierAccess::None;
				barrier.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
				barrier.imageBarrier().resource = image;

				commandBuffer->ResourceBarrier({ barrier });
			}

			commandBuffer->End();
			commandBuffer->ExecuteAndWait();

			RHI::GraphicsContext::GetDefaultAllocator().DestroyBuffer(stagingBuffer);
		}

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(header);
		streamWriter.Write(dataBuffer);
		dataBuffer.Release();

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
		VT_CORE_ASSERT(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		TextureHeader textureHeader{};
		streamReader.Read(textureHeader);
		
		Buffer textureDataBuffer{};
		streamReader.Read(textureDataBuffer);

		Ref<Texture2D> texture = std::reinterpret_pointer_cast<Texture2D>(destinationAsset);

		Ref<RHI::Image2D> image;
		Ref<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();

		// Create image
		{
			RHI::ImageSpecification specification{};
			specification.format = textureHeader.format;
			specification.usage = RHI::ImageUsage::Texture;
			specification.width = textureHeader.mips.front().width;
			specification.height = textureHeader.mips.front().height;
			specification.mips = static_cast<uint32_t>(textureHeader.mips.size());
			specification.generateMips = false;
			specification.debugName = filePath.stem().string();

			image = RHI::Image2D::Create(specification);
		}

		Ref<RHI::Allocation> stagingBuffer = RHI::GraphicsContext::GetDefaultAllocator().CreateBuffer(textureHeader.mips.front().dataSize, RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::CPU);

		// Map memory
		{
			void* bufferPtr = stagingBuffer->Map<void>();;
			memcpy_s(bufferPtr, textureHeader.mips.front().dataSize, textureDataBuffer.As<void>(), textureHeader.mips.front().dataSize);
			stagingBuffer->Unmap();
		}

		commandBuffer->Begin();

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			barrier.imageBarrier().srcStage = RHI::BarrierStage::None;
			barrier.imageBarrier().srcAccess = RHI::BarrierAccess::None;
			barrier.imageBarrier().srcLayout = RHI::ImageLayout::Undefined;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::TransferDestination;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::TransferDestination;
			barrier.imageBarrier().resource = image;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->CopyBufferToImage(stagingBuffer, image, textureHeader.mips.front().width, textureHeader.mips.front().height, 0);
		commandBuffer->End();
		commandBuffer->ExecuteAndWait();

		// Set mip data
		{
			TextureData texData{};
			texData.SetupMips(textureHeader, textureDataBuffer);

			for (size_t i = 1; i < texData.mips.size(); i++)
			{
				commandBuffer->Begin();

				auto mipData = texData.mips.at(i);
				const size_t mipSize = mipData.dataSize;

				// Map mip memory
				{
					void* bufferPtr = stagingBuffer->Map<void>();
					memcpy_s(bufferPtr, mipSize, mipData.dataPtr, mipSize);
					stagingBuffer->Unmap();
				}

				commandBuffer->CopyBufferToImage(stagingBuffer, image, mipData.width, mipData.height, static_cast<uint32_t>(i));
				commandBuffer->End();
				commandBuffer->ExecuteAndWait();
			}
		}

		textureDataBuffer.Release();

		commandBuffer->Begin();
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			barrier.imageBarrier().srcStage = RHI::BarrierStage::Copy;
			barrier.imageBarrier().srcAccess = RHI::BarrierAccess::TransferDestination;
			barrier.imageBarrier().srcLayout = RHI::ImageLayout::TransferDestination;
			barrier.imageBarrier().dstStage = RHI::BarrierStage::PixelShader;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
			barrier.imageBarrier().resource = image;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->End();
		commandBuffer->Execute();

		texture->SetImage(image);

		return true;
	}
}
