#pragma once

#include "Volt/Rendering/GlobalDescriptorSet.h"

#include <unordered_map>

namespace Volt
{
	class GlobalDescriptorSetManager
	{
	public:
		static void Initialize();
		static void Shutdown();

		static Ref<GlobalDescriptorSet> CreateCopyOfType(uint32_t type);
		static std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>> CreateFullCopy();

		static Ref<GlobalDescriptorSet> GetDescriptorSet(uint32_t set);
		static bool HasDescriptorSet(uint32_t set);

	private:
		GlobalDescriptorSetManager() = delete; 
		
		inline static std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>> myDescriptorSets;
	};
}
