#pragma once

#include "RHIModule/Core/RHICommon.h"

#include <CoreUtilities/Math/Hash.h>

namespace Volt::RHI::Utility
{
	inline static const size_t GetHashFromImageSpec(const ImageSpecification& spec, MemoryUsage memoryUsage)
	{
		size_t hash = 0;
		hash = std::hash<uint32_t>()(spec.width);
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(spec.height));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(spec.depth));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(spec.layers));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(spec.mips));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.format)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.usage)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.anisoLevel)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<bool>(spec.isCubeMap)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(memoryUsage)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.imageType)));

		return hash;
	}

	inline static const size_t GetHashFromBufferSpec(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		size_t hash = 0;
		hash = std::hash<size_t>()(size);
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(usage)));
		hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(memoryUsage)));

		return hash;
	}
}
