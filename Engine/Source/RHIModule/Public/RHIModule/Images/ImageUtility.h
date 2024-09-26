#pragma once

#include "RHIModule/Core/RHICommon.h"

namespace Volt::RHI
{
	namespace Utility
	{
		inline static bool IsDepthFormat(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::D16_UNORM:
				case PixelFormat::X8_D24_UNORM_PACK32:
				case PixelFormat::D16_UNORM_S8_UINT:
				case PixelFormat::D24_UNORM_S8_UINT:
				case PixelFormat::D32_SFLOAT_S8_UINT:
				case PixelFormat::D32_SFLOAT:
					return true;
					break;
			}

			return false;
		}

		inline static bool IsStencilFormat(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::D16_UNORM_S8_UINT:
				case PixelFormat::D24_UNORM_S8_UINT:
				case PixelFormat::D32_SFLOAT_S8_UINT:
					return true;
					break;
			}

			return false;
		}

		inline static uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return static_cast<uint32_t>(std::floor(std::log2(std::min(width, height)))) + 1;
		}

		inline uint32_t NextPow2(uint32_t v)
		{
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}

		inline static uint32_t PreviousPOW2(uint32_t value)
		{
			uint32_t r = 1;

			while (r * 2 < value)
			{
				r *= 2;
			}

			return r;
		}

		inline uint32_t IsCompressedFormat(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::BC1_RGB_UNORM_BLOCK:
				case PixelFormat::BC1_RGB_SRGB_BLOCK:
				case PixelFormat::BC1_RGBA_UNORM_BLOCK:
				case PixelFormat::BC1_RGBA_SRGB_BLOCK:
				case PixelFormat::BC2_UNORM_BLOCK:
				case PixelFormat::BC2_SRGB_BLOCK:
				case PixelFormat::BC3_UNORM_BLOCK:
				case PixelFormat::BC3_SRGB_BLOCK:
				case PixelFormat::BC4_UNORM_BLOCK:
				case PixelFormat::BC4_SNORM_BLOCK:
				case PixelFormat::BC5_UNORM_BLOCK:
				case PixelFormat::BC5_SNORM_BLOCK:
				case PixelFormat::BC6H_UFLOAT_BLOCK:
				case PixelFormat::BC6H_SFLOAT_BLOCK:
				case PixelFormat::BC7_UNORM_BLOCK:
				case PixelFormat::BC7_SRGB_BLOCK:
				case PixelFormat::ETC2_R8G8B8_UNORM_BLOCK:
				case PixelFormat::ETC2_R8G8B8_SRGB_BLOCK:
				case PixelFormat::ETC2_R8G8B8A1_UNORM_BLOCK:
				case PixelFormat::ETC2_R8G8B8A1_SRGB_BLOCK:
				case PixelFormat::ETC2_R8G8B8A8_UNORM_BLOCK:
				case PixelFormat::ETC2_R8G8B8A8_SRGB_BLOCK:
				case PixelFormat::EAC_R11_UNORM_BLOCK:
				case PixelFormat::EAC_R11_SNORM_BLOCK:
				case PixelFormat::EAC_R11G11_UNORM_BLOCK:
				case PixelFormat::EAC_R11G11_SNORM_BLOCK:
				case PixelFormat::ASTC_4x4_UNORM_BLOCK:
				case PixelFormat::ASTC_4x4_SRGB_BLOCK:
				case PixelFormat::ASTC_5x4_UNORM_BLOCK:
				case PixelFormat::ASTC_5x4_SRGB_BLOCK:
				case PixelFormat::ASTC_5x5_UNORM_BLOCK:
				case PixelFormat::ASTC_5x5_SRGB_BLOCK:
				case PixelFormat::ASTC_6x5_UNORM_BLOCK:
				case PixelFormat::ASTC_6x5_SRGB_BLOCK:
				case PixelFormat::ASTC_6x6_UNORM_BLOCK:
				case PixelFormat::ASTC_6x6_SRGB_BLOCK:
				case PixelFormat::ASTC_8x5_UNORM_BLOCK:
				case PixelFormat::ASTC_8x5_SRGB_BLOCK:
				case PixelFormat::ASTC_8x6_UNORM_BLOCK:
				case PixelFormat::ASTC_8x6_SRGB_BLOCK:
				case PixelFormat::ASTC_8x8_UNORM_BLOCK:
				case PixelFormat::ASTC_8x8_SRGB_BLOCK:
				case PixelFormat::ASTC_10x5_UNORM_BLOCK:
				case PixelFormat::ASTC_10x5_SRGB_BLOCK:
				case PixelFormat::ASTC_10x6_UNORM_BLOCK:
				case PixelFormat::ASTC_10x6_SRGB_BLOCK:
				case PixelFormat::ASTC_10x8_UNORM_BLOCK:
				case PixelFormat::ASTC_10x8_SRGB_BLOCK:
				case PixelFormat::ASTC_10x10_UNORM_BLOCK:
				case PixelFormat::ASTC_10x10_SRGB_BLOCK:
				case PixelFormat::ASTC_12x10_UNORM_BLOCK:
				case PixelFormat::ASTC_12x10_SRGB_BLOCK:
				case PixelFormat::ASTC_12x12_UNORM_BLOCK:
				case PixelFormat::ASTC_12x12_SRGB_BLOCK:
				case PixelFormat::ASTC_4x4_SFLOAT_BLOCK:
				case PixelFormat::ASTC_5x4_SFLOAT_BLOCK:
				case PixelFormat::ASTC_5x5_SFLOAT_BLOCK:
				case PixelFormat::ASTC_6x5_SFLOAT_BLOCK:
				case PixelFormat::ASTC_6x6_SFLOAT_BLOCK:
				case PixelFormat::ASTC_8x5_SFLOAT_BLOCK:
				case PixelFormat::ASTC_8x6_SFLOAT_BLOCK:
				case PixelFormat::ASTC_8x8_SFLOAT_BLOCK:
				case PixelFormat::ASTC_10x5_SFLOAT_BLOCK:
				case PixelFormat::ASTC_10x6_SFLOAT_BLOCK:
				case PixelFormat::ASTC_10x8_SFLOAT_BLOCK:
				case PixelFormat::ASTC_10x10_SFLOAT_BLOCK:
				case PixelFormat::ASTC_12x10_SFLOAT_BLOCK:
				case PixelFormat::ASTC_12x12_SFLOAT_BLOCK:
					return true;
			}

			return false;
		}

		inline uint32_t GetByteSizePerPixelFromFormat(PixelFormat format)
		{
			switch (format)
			{
				case PixelFormat::R8_UNORM: return 1 * 1;
				case PixelFormat::R16_SFLOAT: return 1 * 2;
				case PixelFormat::R32_SFLOAT: return 1 * 4;
				case PixelFormat::R32_SINT: return 1 * 4;
				case PixelFormat::R32_UINT: return 1 * 4;

				case PixelFormat::R32G32_UINT: return 2 * 4;
				case PixelFormat::R16G16_SFLOAT: return 2 * 2;
				case PixelFormat::R32G32_SFLOAT: return 2 * 4;

				case PixelFormat::R8G8B8A8_UNORM: return 4 * 1;
				case PixelFormat::R16G16B16A16_SFLOAT: return 4 * 2;
				case PixelFormat::R32G32B32A32_SFLOAT: return 4 * 4;

				case PixelFormat::B10G11R11_UFLOAT_PACK32: return 4 * 1;

				case PixelFormat::BC1_RGB_SRGB_BLOCK: return 1;
				case PixelFormat::BC1_RGB_UNORM_BLOCK: return 1;
				case PixelFormat::BC1_RGBA_SRGB_BLOCK: return 1;
				case PixelFormat::BC1_RGBA_UNORM_BLOCK: return 1;

				case PixelFormat::BC2_SRGB_BLOCK: return 1;
				case PixelFormat::BC2_UNORM_BLOCK: return 1;

				case PixelFormat::BC3_SRGB_BLOCK: return 1;
				case PixelFormat::BC3_UNORM_BLOCK: return 1;

				case PixelFormat::BC4_SNORM_BLOCK: return 1;
				case PixelFormat::BC4_UNORM_BLOCK: return 1;

				case PixelFormat::BC5_SNORM_BLOCK: return 1;
				case PixelFormat::BC5_UNORM_BLOCK: return 1;

				case PixelFormat::BC6H_SFLOAT_BLOCK: return 1;
				case PixelFormat::BC6H_UFLOAT_BLOCK: return 1;

				case PixelFormat::BC7_SRGB_BLOCK: return 1;
				case PixelFormat::BC7_UNORM_BLOCK: return 1;
			}

			return 0;
		}
	}
}
