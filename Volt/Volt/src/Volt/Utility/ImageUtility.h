#pragma once

#include "Volt/Rendering/Texture/ImageCommon.h"
#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include <d3d11.h>

namespace Volt
{
	namespace Utility
	{
		///////////////////////////Conversions////////////////////////////
		inline bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::DEPTH16U:
				case ImageFormat::DEPTH32F: return true;
				case ImageFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		inline bool IsTypeless(ImageFormat aFormat)
		{
			switch (aFormat)
			{
				case ImageFormat::R16Typeless:
				case ImageFormat::R32Typeless: 
					return true;
			}

			return false;
		}

		inline bool IsFloatFormat(ImageFormat aFormat)
		{
			switch (aFormat)
			{
				case Volt::ImageFormat::R32F:
				case Volt::ImageFormat::R16F:
				case Volt::ImageFormat::RGBA:
				case Volt::ImageFormat::RGBAS:
				case Volt::ImageFormat::RGBA16F:
				case Volt::ImageFormat::RGBA32F:
				case Volt::ImageFormat::RG16F:
				case Volt::ImageFormat::RG32F:
				case Volt::ImageFormat::SRGB:
					return true;
			}

			return false;
		}

		inline DXGI_FORMAT VoltToDXFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R32F: return DXGI_FORMAT_R32_FLOAT;
				case ImageFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
				case ImageFormat::R32SI: return DXGI_FORMAT_R32_SINT;
				case ImageFormat::R32UI: return DXGI_FORMAT_R32_UINT;
				case ImageFormat::R8U: return DXGI_FORMAT_R8_UNORM;

				case ImageFormat::RGBA: return DXGI_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::RGBAS: return DXGI_FORMAT_R8G8B8A8_SNORM;

				case ImageFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
				case ImageFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;

				case ImageFormat::RG16F: return DXGI_FORMAT_R16G16_FLOAT;
				case ImageFormat::RG32F: return DXGI_FORMAT_R32G32_FLOAT;

				case ImageFormat::SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

				case ImageFormat::BC1: return DXGI_FORMAT_BC1_UNORM;
				case ImageFormat::BC1SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;

				case ImageFormat::BC2: return DXGI_FORMAT_BC2_UNORM;
				case ImageFormat::BC2SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;

				case ImageFormat::BC3: return DXGI_FORMAT_BC3_UNORM;
				case ImageFormat::BC3SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;

				case ImageFormat::BC4: return DXGI_FORMAT_BC4_UNORM;
				case ImageFormat::BC5: return DXGI_FORMAT_BC5_UNORM;

				case ImageFormat::BC7: return DXGI_FORMAT_BC7_UNORM;
				case ImageFormat::BC7SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

				case ImageFormat::R32Typeless: return DXGI_FORMAT_R32_TYPELESS;
				case ImageFormat::R16Typeless: return DXGI_FORMAT_R16_TYPELESS;

				case ImageFormat::DEPTH32F: return DXGI_FORMAT_D32_FLOAT;
				case ImageFormat::DEPTH16U: return DXGI_FORMAT_D16_UNORM;
				case ImageFormat::DEPTH24STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;

				default:
					break;
			}

			return (DXGI_FORMAT)0;
		}

		inline D3D11_FILTER VoltToDXFilter(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::None: VT_CORE_ASSERT(false, "Filter must be chosen!"); return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureFilter::Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureFilter::Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureFilter::Anisotopy: return D3D11_FILTER_ANISOTROPIC;
			}

			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}

		//inline D3D11_TEXTURE_ADDRESS_MODE LampToVulkanWrap(TextureWrap wrap)
		//{
		//	switch (wrap)
		//	{
		//		case TextureWrap::None: LP_CORE_ASSERT(false, "Wrap mode must be set!"); break;
		//		case TextureWrap::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		//		case TextureWrap::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//	}
		//	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		//}

		inline uint32_t PerPixelSizeFromFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return 0;
				case ImageFormat::R32F: return 1 * 4;
				case ImageFormat::R32SI: return 1 * 4;
				case ImageFormat::R32UI: return 1 * 4;
				case ImageFormat::RGBA: return 4 * 1;
				case ImageFormat::RGBA16F: return 4 * 2;
				case ImageFormat::RGBA32F: return 4 * 4;
				case ImageFormat::RG16F: return 2 * 2;
				case ImageFormat::RG32F: return 2 * 4;
				case ImageFormat::SRGB: return 4 * 1;
				case ImageFormat::DEPTH32F: return 1 * 4;
				case ImageFormat::DEPTH24STENCIL8: return 4;
			}

			return 0;
		}

		static uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return static_cast<uint32_t>(std::floor(std::log2(std::min(width, height)))) + 1;
		}

		static D3D11_RENDER_TARGET_BLEND_DESC GetBlendDescFromBlendType(TextureBlend aBlending)
		{
			D3D11_RENDER_TARGET_BLEND_DESC blendDesc{};
			blendDesc.BlendEnable = true;
			blendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
			blendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			switch (aBlending)
			{
				case TextureBlend::None:
				{
					blendDesc.BlendEnable = false;
					break;
				}

				case TextureBlend::Alpha:
				{
					blendDesc.BlendEnable = true;
					blendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
					blendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
					blendDesc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
					blendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
					blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					break;
				}

				case TextureBlend::Add:
				{
					blendDesc.BlendEnable = true;
					blendDesc.SrcBlend = D3D11_BLEND_ONE;
					blendDesc.DestBlend = D3D11_BLEND_ONE;
					blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
					blendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
					blendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
					blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					break;
				}

				case TextureBlend::ZeroSrcColor:
				{
					blendDesc.BlendEnable = true;
					blendDesc.SrcBlend = D3D11_BLEND_ZERO;
					blendDesc.DestBlend = D3D11_BLEND_SRC_COLOR;
					blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
					blendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
					blendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
					blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
					blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
					break;
				}
			}

			return blendDesc;
		}
	}
}