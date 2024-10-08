#pragma once

#include "VulkanRHIModule/Descriptors/VulkanDescriptorCommon.h"

#include <RHIModule/Descriptors/DescriptorTable.h>

#include <CoreUtilities/Pointers/WeakPtr.h>
#include <CoreUtilities/Containers/Vector.h>

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

		void InitilizeWriteDescriptor(DescriptorWrite& writeDescriptor, const uint32_t binding, const uint32_t descriptorType, VkDescriptorSet_T* dstDescriptorSet);

		WeakPtr<Shader> m_shader;
		bool m_isDirty = false;

		Vector<DescriptorWrite> m_descriptorWrites;
		Vector<DescriptorWrite> m_activeDescriptorWrites;

		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DescriptorImageInfo>>> m_imageDescriptorInfos; // Set -> Binding -> Array Index
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DescriptorBufferInfo>>> m_bufferDescriptorInfos; // Set -> Binding -> Array Index
		std::map<uint32_t, std::map<uint32_t, uint32_t>> m_writeDescriptorsMapping; // Set -> Binding
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, DefaultInvalid>>> m_activeDescriptorWritesMapping; // Set -> Binding -> ArrayIndex 

		VkDescriptorPool_T* m_descriptorPool = nullptr;
		std::map<uint32_t, VkDescriptorSet_T*> m_descriptorSets;
	};
}
