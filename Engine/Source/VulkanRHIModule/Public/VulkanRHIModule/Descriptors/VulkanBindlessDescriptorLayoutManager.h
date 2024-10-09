#pragma once

#include <CoreUtilities/Containers/Vector.h>

struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;

namespace Volt::RHI
{
	class VulkanBindlessDescriptorLayoutManager
	{
	public:
		inline static constexpr uint32_t CBV_SRV_UAV_BINDING = 0;
		inline static constexpr uint32_t SAMPLERS_BINDING = 1;

		static void CreateGlobalDescriptorLayout();
		static void DestroyGlobalDescriptorLayout();

		static std::array<VkDescriptorSetLayout_T*, 2> GetGlobalDescriptorSetLayouts();

	private:
		static bool TryCreateMutableDescriptorSetLayout(VkDescriptorSetLayout_T*& outDescriptorSetLayouts);

		VulkanBindlessDescriptorLayoutManager() = delete;
	};
}
