#pragma once

#include "VulkanRHIModule/Descriptors/VulkanDescriptorCommon.h"

#include <RHIModule/Descriptors/BindlessDescriptorTable.h>
#include <RHIModule/Descriptors/ResourceRegistry.h>

#include <CoreUtilities/Containers/FunctionQueue.h>
#include <CoreUtilities/Containers/ThreadSafeVector.h>

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

		void Bind(CommandBuffer& commandBuffer, WeakPtr<UniformBuffer> constantsBuffer, const uint32_t offsetIndex, const uint32_t stride) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();
		void Invalidate();

		void PrepareHeapForRender();
		void PrepareDefaultForRender();

		VkDescriptorSet_T* GetOrAllocateConstantsSet();
		void WriteConstantsSet(VkDescriptorSet_T* dstSet, WeakPtr<UniformBuffer> constantsBuffer);

		VkDescriptorSet_T* GetCurrentMainDescriptorSet() const;

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
		Vector<VkDescriptorSet_T*> m_mainDescriptorSets;

		ThreadSafeVector<VkDescriptorSet_T*> m_availiableConstantsSet;
		std::mutex m_descriptorAllocationMutex;
		std::mutex m_writeDescriptorMutex;

		uint64_t m_frameIndex = 0;
		uint64_t m_framesInFlight = 0;

		// VK_EXT_mutable_descriptor_type
		ResourceRegistry m_heapRegistry;
		bool m_useHeapRegistry = false;
	};
}
