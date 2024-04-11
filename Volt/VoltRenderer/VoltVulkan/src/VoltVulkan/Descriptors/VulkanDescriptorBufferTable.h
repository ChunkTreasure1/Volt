#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <CoreUtilities/Buffer/Buffer.h>

#include <vector>

namespace Volt::RHI
{
	struct DescriptorTypeOffsets
	{
		uint64_t uboOffset;
		uint64_t ssboOffset;
		uint64_t storageImageOffset;
		uint64_t imageOffset;
		uint64_t samplerOffset;

		uint64_t uboSize;
		uint64_t ssboSize;
		uint64_t storageImageSize;
		uint64_t imageSize;
		uint64_t samplerSize;
	};

	class Allocation;
	class VulkanDescriptorBufferTable : public DescriptorTable
	{
	public:
		VulkanDescriptorBufferTable(const DescriptorTableCreateInfo& createInfo);
		~VulkanDescriptorBufferTable() override;

		void SetImageView(Ref<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetBufferView(Ref<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;

		void SetImageView(std::string_view name, Ref<ImageView> view, uint32_t arrayIndex = 0) override;
		void SetBufferView(std::string_view name, Ref<BufferView> view, uint32_t arrayIndex = 0) override;
		void SetSamplerState(std::string_view name, Ref<SamplerState> samplerState, uint32_t arrayIndex = 0) override;

		void SetBufferViews(const std::vector<Ref<BufferView>>& bufferViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset /* = 0 */) override;
		void SetImageViews(const std::vector<Ref<ImageView>>& imageViews, uint32_t set, uint32_t binding, uint32_t arrayStartOffset /* = 0 */) override;

		void SetSamplerState(Ref<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex /* = 0 */) override;

		void Update(const uint32_t index /* = 0 */) override;

	protected:
		friend class VulkanCommandBuffer;

		void* GetHandleImpl() const override;
		void Bind(CommandBuffer& commandBuffer) override;

	private:
		void Invalidate();
		void Release();

		void CalculateDescriptorOffsets();

		Weak<Shader> m_shader;
		uint32_t m_descriptorBufferCount = 0;
		uint64_t m_accumulatedSize = 0;

		Ref<Allocation> m_descriptorBuffer;
		Buffer m_hostDescriptorBuffer;
		DescriptorTypeOffsets m_descriptorTypeOffsets{};

		std::vector<uint64_t> m_descriptorSetLayoutSizes{};

		std::vector<uint32_t> m_usedDescriptorSets;
		std::map<uint32_t, uint64_t> m_descriptorSetOffsets{};

		std::map<uint32_t, std::map<uint32_t, uint64_t>> m_descriptorSetBindingOffsets;
		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_imageDescriptorTypes;
	};

	VTVK_API Ref<DescriptorTable> CreateVulkanDescriptorBufferTable(const DescriptorTableCreateInfo& createInfo);
}
