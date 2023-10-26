#pragma once

struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;

namespace Volt::RHI
{
	class VulkanBindlessManager
	{
	public:
		static void CreateGlobalDescriptorLayout();
		static void DestroyGlobalDescriptorLayout();

		static VkDescriptorSetLayout_T* GetGlobalDescriptorSetLayout();
		static VkDescriptorSet_T* GetGlobalDescriptorSet();

	private:

		VulkanBindlessManager() = delete;
	};
}
