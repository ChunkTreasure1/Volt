#pragma once

#include "VoltRHI/Core/RHICommon.h"

namespace Volt::RHI
{
	namespace Utility
	{
		inline static bool IsDepthFormat(Format format)
		{
			switch (format)
			{
				case Format::D16_UNORM:
				case Format::X8_D24_UNORM_PACK32:
				case Format::D16_UNORM_S8_UINT:
				case Format::D24_UNORM_S8_UINT:
				case Format::D32_SFLOAT_S8_UINT:
				case Format::D32_SFLOAT:
					return true;
					break;
			}

			return false;
		}

		inline static bool IsStencilFormat(Format format)
		{
			switch (format)
			{
				case Format::D16_UNORM_S8_UINT:
				case Format::D24_UNORM_S8_UINT:
				case Format::D32_SFLOAT_S8_UINT:
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
	}
}
