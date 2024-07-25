#pragma once

#include "VoltVulkan/Descriptors/VulkanDescriptorCommon.h"

#include <VoltRHI/Descriptors/BindlessDescriptorTable.h>
#include <VoltRHI/Descriptors/ResourceRegistry.h>

#include <CoreUtilities/Containers/FunctionQueue.h>

#include <list>

struct VkDescriptorPool_T;
struct VkDescriptorSet_T;

namespace Volt::RHI
{
	class VulkanBindlessDescriptorTable : public BindlessDescriptorTable
	{
	public:
		VulkanBindlessDescriptorTable(uint64_t framesInFlight);
		~VulkanBindlessDescriptorTable() override;

		ResourceHandle RegisterBuffer(WeakPtr<StorageBuffer> storageBuffer) override;
		ResourceHandle RegisterImageView(WeakPtr<ImageView> imageView) override;
		ResourceHandle RegisterSamplerState(WeakPtr<SamplerState> samplerState) override;

		void UnregisterBuffer(ResourceHandle handle) override;
		void UnregisterImageView(ResourceHandle handle, ImageViewType viewType) override;
		void UnregisterSamplerState(ResourceHandle handle) override;

		void MarkBufferAsDirty(ResourceHandle handle) override;
		void MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType) override;
		void MarkSamplerStateAsDirty(ResourceHandle handle) override;

		ResourceHandle GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer) override;

		void Update() override;
		void PrepareForRender() override;

		void SetOffsetIndexAndStride(const uint32_t offsetIndex, const uint32_t stride) override;
		void SetConstantsBuffer(WeakPtr<UniformBuffer> constantsBuffer) override;
		void Bind(CommandBuffer& commandBuffer) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();
		void Invalidate();

		VkDescriptorSet_T* GetOrAllocateConstantsSet();
		void WriteConstantsSet(VkDescriptorSet_T* dstSet);

		ResourceRegistry m_image2DRegistry;
		ResourceRegistry m_image2DArrayRegistry;
		ResourceRegistry m_image3DRegistry;
		ResourceRegistry m_imageCubeRegistry;

		ResourceRegistry m_bufferRegistry;
		ResourceRegistry m_samplerRegistry;

		std::list<DescriptorImageInfo> m_activeDescriptorImageInfos;
		std::list<DescriptorBufferInfo> m_activeDescriptorBufferInfos;

		Vector<DescriptorWrite> m_activeDescriptorWrites;

		VkDescriptorPool_T* m_descriptorPool = nullptr;
		VkDescriptorSet_T* m_mainDescriptorSet = nullptr;

		Vector<VkDescriptorSet_T*> m_availiableConstantsSet;

		uint32_t m_offsetIndex = 0;
		uint32_t m_offsetStride = 0;
		WeakPtr<UniformBuffer> m_constantsBuffer;
	};
}
