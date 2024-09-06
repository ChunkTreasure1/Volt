#pragma once

#include "VulkanRHIModule/Core.h"

#include <RHIModule/Descriptors/DescriptorTable.h>
#include <CoreUtilities/Buffer/Buffer.h>
#include <CoreUtilities/Pointers/WeakPtr.h>



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

		void SetImageView(WeakPtr<ImageView> imageView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetBufferView(WeakPtr<BufferView> bufferView, uint32_t set, uint32_t binding, uint32_t arrayIndex = 0) override;
		void SetSamplerState(WeakPtr<SamplerState> samplerState, uint32_t set, uint32_t binding, uint32_t arrayIndex /* = 0 */) override;

		void SetImageView(std::string_view name, WeakPtr<ImageView> view, uint32_t arrayIndex = 0) override;
		void SetBufferView(std::string_view name, WeakPtr<BufferView> view, uint32_t arrayIndex = 0) override;
		void SetSamplerState(std::string_view name, WeakPtr<SamplerState> samplerState, uint32_t arrayIndex = 0) override;

		void PrepareForRender() override;

	protected:
		friend class VulkanCommandBuffer;

		void* GetHandleImpl() const override;
		void Bind(CommandBuffer& commandBuffer) override;

	private:
		void Invalidate();
		void Release();

		void CalculateDescriptorOffsets();

		WeakPtr<Shader> m_shader;
		uint32_t m_descriptorBufferCount = 0;
		uint64_t m_accumulatedSize = 0;

		RefPtr<Allocation> m_descriptorBuffer;
		Buffer m_hostDescriptorBuffer;
		DescriptorTypeOffsets m_descriptorTypeOffsets{};

		Vector<uint64_t> m_descriptorSetLayoutSizes{};

		Vector<uint32_t> m_usedDescriptorSets;
		std::map<uint32_t, uint64_t> m_descriptorSetOffsets{};

		std::map<uint32_t, std::map<uint32_t, uint64_t>> m_descriptorSetBindingOffsets;
		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_imageDescriptorTypes;
	};
}
