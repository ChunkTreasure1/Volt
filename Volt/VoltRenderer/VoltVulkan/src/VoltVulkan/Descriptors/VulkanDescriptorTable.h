#pragma once

#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

struct VkDescriptorPool_T;
struct VkDescriptorSet_T;
struct VkBufferView_T;

struct VkSampler_T;
struct VkImageView_T;
struct VkBuffer_T;

struct VkDescriptorImageInfo;
struct VkDescriptorBufferInfo;

namespace Volt::RHI
{
	struct WriteDescriptor2
	{
		uint32_t sType = 0;
		const void* pNext = nullptr;
		VkDescriptorSet_T* dstSet = nullptr;
		uint32_t dstBinding = 0;
		uint32_t dstArrayElement = 0;
		uint32_t descriptorCount = 0;
		uint32_t descriptorType = 0;
		const VkDescriptorImageInfo* pImageInfo;
		const VkDescriptorBufferInfo* pBufferInfo;
		const VkBufferView_T* pTexelBufferView;
	};

	struct DescriptorImageInfo2
	{
		VkSampler_T* sampler = nullptr;
		VkImageView_T* imageView = nullptr;
		uint32_t imageLayout = 0;
	};

	struct DescriptorBufferInfo2
	{
		VkBuffer_T* buffer = nullptr;
		uint64_t offset = 0;
		uint64_t range = 0;
	};

	class VulkanDescriptorTable : public DescriptorTable
	{
	public:
		VulkanDescriptorTable(const DescriptorTableCreateInfo& createInfo);
		~VulkanDescriptorTable() override;

		void SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0 ) override;
		void SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex /* = 0 */) override;

		void SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex = 0) override;
		void SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex = 0) override;
		void SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex = 0) override;

		void PrepareForRender() override;
		void Bind(CommandBuffer& commandBuffer) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		struct DefaultInvalid
		{
			inline static constexpr uint32_t INVALID_VALUE = std::numeric_limits<uint32_t>::max();
			uint32_t value = INVALID_VALUE;
		};

		void Invalidate();
		void Release();

		void BuildWriteDescriptors();
		void InitializeInfoStructs();

		void InitilizeWriteDescriptor(WriteDescriptor2& writeDescriptor, const uint32_t binding, const uint32_t descriptorType, VkDescriptorSet_T* dstDescriptorSet);

		WeakPtr<Shader> m_shader;
		bool m_isGlobal = false;
		bool m_isDirty = false;

		std::vector<WriteDescriptor2> m_writeDescriptors;
		std::vector<WriteDescriptor2> m_activeWriteDescriptors;

		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DescriptorImageInfo2>>> m_imageDescriptorInfos; // Set -> Binding -> Array Index
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DescriptorBufferInfo2>>> m_bufferDescriptorInfos; // Set -> Binding -> Array Index
		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_writeDescriptorsMapping; // Set -> Binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DefaultInvalid>>> m_activeWriteDescriptorsMapping; // Set -> Binding -> ArrayIndex 

		VkDescriptorPool_T* m_descriptorPool = nullptr;
		std::map<uint32_t, VkDescriptorSet_T*> m_descriptorSets;
	};
}
