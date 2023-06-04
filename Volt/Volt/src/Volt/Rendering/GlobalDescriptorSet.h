#pragma once

#include "Volt/Rendering/Shader/ShaderCommon.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	enum class DescriptorType
	{
		Texture = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,

		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,

		UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	};

	struct DescriptorBinding
	{
		uint32_t binding = 0;
		uint32_t descriptorCount = 1;
		DescriptorType descriptorType = DescriptorType::UniformBuffer;
		ShaderStage stageFlags = ShaderStage::None;
	};

	struct GlobalDescriptorSetSpecification
	{
		uint32_t set;
		std::vector<DescriptorBinding> descriptorBindings;

		bool updateAfterBind = false;
		uint32_t descriptorSetCount = 1;
	};

	class GlobalDescriptorSet
	{
	public:
		GlobalDescriptorSet(const GlobalDescriptorSetSpecification& specification);
		~GlobalDescriptorSet();

		VkDescriptorSet GetOrAllocateDescriptorSet(uint32_t index);
		VkWriteDescriptorSet GetWriteDescriptor(uint32_t index, uint32_t binding, uint32_t arrayElement = 0);

		inline const VkDescriptorSetLayout GetDescriptorSetLayout() const { return myDescriptorSetLayout; }
		inline const GlobalDescriptorSetSpecification& GetSpecification() const { return mySpecification; }

		static Ref<GlobalDescriptorSet> Create(const GlobalDescriptorSetSpecification& specification);

	private:
		void CreateDescriptorPools();

		struct CountType
		{	
			uint32_t count = 0;
		};

		GlobalDescriptorSetSpecification mySpecification;
		VkDescriptorSetLayout myDescriptorSetLayout = nullptr;

		std::unordered_map<uint32_t, std::vector<VkWriteDescriptorSet>> myWriteDescriptors;
		std::unordered_map<VkDescriptorType, CountType> myPoolSizes;
		
		uint32_t myMaxTotalDescriptorSets = 0;

		std::vector<VkDescriptorPool> myDescriptorPools;
		std::vector<VkDescriptorSet> myDescriptorSets;

		std::mutex myAllocateDescriptorSetMutex;
	};
}
