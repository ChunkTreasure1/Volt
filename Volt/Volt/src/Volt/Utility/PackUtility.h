#pragma once

#include <glm/glm.hpp>

namespace Volt
{
	namespace Utility
	{
		inline const uint32_t PackUNormFloat4AsUInt(const glm::vec4& value)
		{
			uint32_t result = 0;

			uint32_t packedX = static_cast<uint32_t>(value.x * 255.f + 0.5f);
			uint32_t packedY = static_cast<uint32_t>(value.y * 255.f + 0.5f);
			uint32_t packedZ = static_cast<uint32_t>(value.z * 255.f + 0.5f);
			uint32_t packedW = static_cast<uint32_t>(value.w * 255.f + 0.5f);

			result |= (packedX << 24);
			result |= (packedY << 16);
			result |= (packedZ << 8);
			result |= packedW;

			return result;
		}
	
		inline const glm::vec4 UnpackUIntToUNormFloat4(const uint32_t value)
		{
			glm::vec4 result = 0.f;

			const uint32_t packedX = (value >> 24) & 0xFF;
			const uint32_t packedY = (value >> 16) & 0xFF;
			const uint32_t packedZ = (value >> 8) & 0xFF;
			const uint32_t packedW = value & 0xFF;
		
			result.x = static_cast<float>(packedX) / 255.f;
			result.y = static_cast<float>(packedY) / 255.f;
			result.z = static_cast<float>(packedZ) / 255.f;
			result.w = static_cast<float>(packedW) / 255.f;

			return result;
		}
	}
}
