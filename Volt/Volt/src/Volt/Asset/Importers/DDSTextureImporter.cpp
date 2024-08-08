#include "vtpch.h"
#include "DDSTextureImporter.h"

#include "Volt/Rendering/Texture/Texture2D.h"

#include <RHIModule/Images/Image2D.h>
#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Graphics/GraphicsContext.h>

#include <RHIModule/Memory/Allocation.h>
#include <RHIModule/Utility/ResourceUtility.h>

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
				case tdl::ErrorFileOpen: VT_LOG(Error, "Unable to open texture {0}!", path.string().c_str()); break;
				case tdl::ErrorRead: VT_LOG(Error, "Unable to read texture {0}!", path.string().c_str()); break;
				case tdl::ErrorMagicWord: VT_LOG(Error, "Unable to read magic word in texture {0}!", path.string().c_str()); break;
				case tdl::ErrorSize: VT_LOG(Error, "Size is wrong in texture {0}!", path.string().c_str()); break;
				case tdl::ErrorVerify: VT_LOG(Error, "Unable to verify texture {0}!", path.string().c_str()); break;
				case tdl::ErrorNotSupported: VT_LOG(Error, "Texture type of texture {0} not supported!", path.string().c_str()); break;
				case tdl::ErrorInvalidData: VT_LOG(Error, "Invalid data in texture {0}!", path.string().c_str()); break;
			}
		}

		static RHI::PixelFormat DDSToImageFormat(tdl::DDSFile::DXGIFormat format)
		{
			switch (format)
			{
				case tdl::DDSFile::DXGIFormat::R32G32B32A32_Float: return RHI::PixelFormat::R32G32B32A32_SFLOAT;
				case tdl::DDSFile::DXGIFormat::R16G16B16A16_Float: return RHI::PixelFormat::R16G16B16A16_SFLOAT;

				case tdl::DDSFile::DXGIFormat::R32G32_Float: return RHI::PixelFormat::R32G32_SFLOAT;
				case tdl::DDSFile::DXGIFormat::R16G16_Float: return RHI::PixelFormat::R16G16_SFLOAT;

				case tdl::DDSFile::DXGIFormat::R8G8B8A8_UNorm: return RHI::PixelFormat::R8G8B8A8_UNORM;
				case tdl::DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB: return RHI::PixelFormat::R8G8B8A8_SRGB;

				case tdl::DDSFile::DXGIFormat::R32_Float: return RHI::PixelFormat::R32_SFLOAT;
				case tdl::DDSFile::DXGIFormat::R32_UInt: return RHI::PixelFormat::R32_UINT;
				case tdl::DDSFile::DXGIFormat::R32_SInt: return RHI::PixelFormat::R32_SINT;

				case tdl::DDSFile::DXGIFormat::BC1_UNorm: return RHI::PixelFormat::BC1_RGB_UNORM_BLOCK;
				case tdl::DDSFile::DXGIFormat::BC1_UNorm_SRGB: return RHI::PixelFormat::BC1_RGB_SRGB_BLOCK;

				case tdl::DDSFile::DXGIFormat::BC2_UNorm: return RHI::PixelFormat::BC2_UNORM_BLOCK;
				case tdl::DDSFile::DXGIFormat::BC2_UNorm_SRGB: return RHI::PixelFormat::BC2_SRGB_BLOCK;

				case tdl::DDSFile::DXGIFormat::BC3_UNorm: return RHI::PixelFormat::BC3_UNORM_BLOCK;
				case tdl::DDSFile::DXGIFormat::BC3_UNorm_SRGB: return RHI::PixelFormat::BC3_SRGB_BLOCK;

				case tdl::DDSFile::DXGIFormat::BC4_UNorm: return RHI::PixelFormat::BC4_UNORM_BLOCK;

				case tdl::DDSFile::DXGIFormat::BC5_UNorm: return RHI::PixelFormat::BC5_UNORM_BLOCK;

				case tdl::DDSFile::DXGIFormat::BC7_UNorm: return RHI::PixelFormat::BC7_UNORM_BLOCK;
				case tdl::DDSFile::DXGIFormat::BC7_UNorm_SRGB: return RHI::PixelFormat::BC7_SRGB_BLOCK;
			}

			return RHI::PixelFormat::R8G8B8A8_UNORM;
		}
	}

	bool DDSTextureImporter::ImportTextureImpl(const std::filesystem::path& path, Texture2D& outTexture)
	{
		tdl::DDSFile dds;
		auto returnCode = dds.Load(path.string().c_str());
		if (returnCode != tdl::Result::Success)
		{
			Utility::PrintDDSError(returnCode, path);
		}

		if (dds.GetTextureDimension() != tdl::DDSFile::TextureDimension::Texture2D)
		{
			VT_LOG(Error, "Texture {0} is not 2D!", path.string().c_str());
			return false;
		}

		auto imageData = dds.GetImageData();

		const uint32_t width = imageData->m_width;
		const uint32_t height = imageData->m_height;

		RefPtr<RHI::Image2D> image;
		RefPtr<RHI::CommandBuffer> commandBuffer = RHI::CommandBuffer::Create();

		// Create image
		{
			RHI::ImageSpecification specification{};
			specification.format = Utility::DDSToImageFormat(dds.GetFormat());
			specification.usage = RHI::ImageUsage::Texture;
			specification.width = width;
			specification.height = height;
			specification.mips = dds.GetMipCount();
			specification.generateMips = false;
			specification.debugName = path.stem().string();

			image = RHI::Image2D::Create(specification);
		}

		RHI::ImageCopyData copyData{};
		for (uint32_t i = 0; i < dds.GetMipCount(); i++)
		{
			auto mipData = dds.GetImageData(i);
			
			auto& subData = copyData.copySubData.emplace_back();
			subData.data = mipData->m_mem;
			subData.rowPitch = mipData->m_memPitch;
			subData.slicePitch = mipData->m_memSlicePitch;
			subData.width = mipData->m_width;
			subData.height = mipData->m_height;
			subData.depth = mipData->m_depth;
			subData.subResource.baseArrayLayer = 0;
			subData.subResource.baseMipLevel = i;
			subData.subResource.layerCount = 1;
			subData.subResource.levelCount = 1;
		}

		commandBuffer->Begin();

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), image);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::Copy;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::CopyDest;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::CopyDest;
			barrier.imageBarrier().resource = image;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->UploadTextureData(image, copyData);
		
		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = RHI::BarrierType::Image;
			RHI::ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), image);

			barrier.imageBarrier().dstStage = RHI::BarrierStage::PixelShader;
			barrier.imageBarrier().dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.imageBarrier().dstLayout = RHI::ImageLayout::ShaderRead;
			barrier.imageBarrier().resource = image;

			commandBuffer->ResourceBarrier({ barrier });
		}

		commandBuffer->End();
		commandBuffer->Execute();

		outTexture.SetImage(image);

		return true;
	}
}
