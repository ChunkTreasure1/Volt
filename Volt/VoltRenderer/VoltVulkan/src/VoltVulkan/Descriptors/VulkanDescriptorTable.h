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

		void SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetBufferViewSet(Ref<BufferViewSet> bufferViewSet, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;

		void SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex /* = 0 */) override;

		void Update(const uint32_t index);

		inline const std::map<uint32_t, std::vector<VkDescriptorSet_T*>>& GetDescriptorSets() const { return m_descriptorSets; }

	protected:
		void* GetHandleImpl() const override;

	private:
		void SetDirty(bool state);

		void Invalidate();
		void Release();

		void BuildWriteDescriptors();
		void InitializeInfoStructs();

		Weak<Shader> m_shader;
		uint32_t m_maxTotalDescriptorCount = 0;

		std::map<uint32_t, std::vector<VkDescriptorSet_T*>> m_descriptorSets{};
		std::vector<VkDescriptorPool_T*> m_descriptorPools;

		std::vector<std::vector<WriteDescriptor>> m_writeDescriptors;
		std::vector<std::vector<WriteDescriptor>> m_activeWriteDescriptors;

		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorImageInfo>>>> m_imageInfos; // Set -> Binding -> Array Index -> Frames
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, std::vector<DescriptorBufferInfo>>>> m_bufferInfos; // Set -> Binding -> Array Index -> Frames

		std::map<uint32_t, std::map<uint32_t, std::vector<uint32_t>>> m_writeDescriptorsMapping; // Set -> Binding -> Array Index -> Frames
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, std::vector<uint32_t>>>> m_activeWriteDescriptorsMapping; // Set -> Binding -> Array Index -> Frames

		std::vector<bool> m_isDirty;
	};
}
