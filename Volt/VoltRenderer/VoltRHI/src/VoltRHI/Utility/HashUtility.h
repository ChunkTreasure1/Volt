#pragma once

#include "VoltRHI/Core/RHICommon.h"
#include "VoltRHI/Memory/MemoryUtility.h"

namespace Volt::RHI::Utility
{
	inline static const size_t GetHashFromImageSpec(const ImageSpecification& spec, MemoryUsage memoryUsage)
	{
		size_t hash = 0;
		hash = std::hash<uint32_t>()(spec.width);
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(spec.height));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(spec.depth));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(spec.layers));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(spec.mips));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.format)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.usage)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.anisoLevel)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<bool>(spec.isCubeMap)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(memoryUsage)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(spec.imageType)));

		return hash;
	}

	inline static const size_t GetHashFromBufferSpec(const size_t size, BufferUsage usage, MemoryUsage memoryUsage)
	{
		size_t hash = 0;
		hash = std::hash<size_t>()(size);
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(usage)));
		hash = Utility::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(memoryUsage)));

		return hash;
	}
}
