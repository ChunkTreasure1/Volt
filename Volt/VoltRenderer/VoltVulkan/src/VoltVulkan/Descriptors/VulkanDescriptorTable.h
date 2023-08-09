#pragma once

#include <VoltRHI/Descriptors/DescriptorTable.h>

struct VkDescriptorPool_T;
struct VkDescriptorSet_T;
struct VkBufferView_T;

struct VkSampler_T;
struct VkImageView_T;
struct VkBuffer_T;

namespace Volt::RHI
{
	struct WriteDescriptor
	{
		uint32_t sType = 0;
		const void* pNext = nullptr;
		VkDescriptorSet_T* dstSet = nullptr;
		uint32_t dstBinding = 0;
		uint32_t dstArrayElement = 0;
		uint32_t descriptorCount = 0;
		uint32_t descriptorType = 0;
		const struct VkDescriptorImageInfo* pImageInfo;
		const struct VkDescriptorBufferInfo* pBufferInfo;
		const VkBufferView_T* pTexelBufferView;
	};

	struct DescriptorImageInfo
	{
		VkSampler_T* sampler = nullptr;
		VkImageView_T* imageView = nullptr;
		uint32_t imageLayout = 0;
	};

	struct DescriptorBufferInfo
	{
		VkBuffer_T* buffer = nullptr;
		uint64_t offset = 0;
		uint64_t range = 0;
	};

	class VulkanDescriptorTable : public DescriptorTable
	{
	public:
		VulkanDescriptorTable(const DescriptorTableSpecification& specification);
		~VulkanDescriptorTable() override;

		void SetImageView(uint32_t set, uint32_t binding, Ref<ImageView> image) override;

		void Update(const uint32_t index);

		inline const std::map<uint32_t, std::vector<VkDescriptorSet_T*>>& GetDescriptorSets() const { return m_descriptorSets; }

	protected:
		void* GetHandleImpl() override;

	private:
		void Invalidate();
		void Release();
		void BuildWriteDescriptors();
	
		Weak<Shader> m_shader;
		uint32_t m_maxTotalDescriptorCount = 0;

		std::map<uint32_t, std::vector<VkDescriptorSet_T*>> m_descriptorSets{};
		std::vector<VkDescriptorPool_T*> m_descriptorPools;
		std::vector<std::vector<WriteDescriptor>> m_writeDescriptors;

		std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorImageInfo>>> m_imageInfos;
		std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorBufferInfo>>> m_bufferInfos;

		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_writeDescriptorsMapping;

		std::vector<bool> m_isDirty;
	};
}
