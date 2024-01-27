#pragma once

#include "VoltRHI/Core/RHICommon.h"

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

		inline static uint32_t PreviousPOW2(uint32_t value)
		{
			uint32_t r = 1;

			while (r * 2 < value)
			{
				r *= 2;
			}

			return r;
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
			}

			return 0;
		}
	}
}
