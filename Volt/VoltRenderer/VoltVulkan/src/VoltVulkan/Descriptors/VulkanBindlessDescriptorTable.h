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
		VulkanBindlessDescriptorTable();
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

		void Bind(CommandBuffer& commandBuffer) override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Release();
		void Invalidate();

		ResourceRegistry m_image2DRegistry;
		ResourceRegistry m_image2DArrayRegistry;
		ResourceRegistry m_imageCubeRegistry;

		ResourceRegistry m_bufferRegistry;
		ResourceRegistry m_samplerRegistry;

		std::list<DescriptorImageInfo> m_activeDescriptorImageInfos;
		std::list<DescriptorBufferInfo> m_activeDescriptorBufferInfos;

		std::vector<DescriptorWrite> m_activeDescriptorWrites;

		VkDescriptorPool_T* m_descriptorPool = nullptr;
		VkDescriptorSet_T* m_descriptorSet = nullptr;
	};
}
