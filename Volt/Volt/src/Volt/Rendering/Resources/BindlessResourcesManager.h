#pragma once

#include "Volt/Rendering/Resources/ResourceHandle.h"
#include "Volt/Utility/FunctionQueue.h"

#include <VoltRHI/Core/RHIResource.h>
#include <VoltRHI/Descriptors/DescriptorTable.h>
#include <CoreUtilities/Weak.h>

#include <span>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class Image2D;
		class SamplerState;
	}

	struct RegisteredResource
	{
		ResourceHandle handle;
		uint32_t referenceCount = 0;
		RHI::ImageUsage imageUsage;

		WeakPtr<RHI::RHIInterface> resource;
	};

	class ResourceRegistry
	{
	public:
		ResourceRegistry();

		ResourceHandle RegisterResource(WeakPtr<RHI::RHIInterface> resource, RHI::ImageUsage imageUsage = RHI::ImageUsage::None);
		void UnregisterResource(ResourceHandle handle);

		ResourceHandle GetResourceHandle(WeakPtr<RHI::RHIInterface> resource);

		void Update();
		void MarkAsDirty(ResourceHandle handle);
		void ClearDirtyResources();

		VT_INLINE VT_NODISCARD std::mutex& GetMutex() { return m_mutex; }
		VT_INLINE VT_NODISCARD std::span<const ResourceHandle> GetDirtyResources() const { return m_dirtyResources; }
		VT_INLINE VT_NODISCARD const RegisteredResource& GetResource(ResourceHandle resourceHandle) const { return m_resources.at(resourceHandle); }

	private:
		friend class BindlessResourcesManager;

		inline static constexpr uint64_t FRAME_COUNT = 2;

		std::vector<RegisteredResource> m_resources;
		std::vector<ResourceHandle> m_vacantResourceHandles;
		std::vector<ResourceHandle> m_dirtyResources;
		std::vector<FunctionQueue> m_removalQueue;

		std::unordered_map<size_t, ResourceHandle> m_resourceHashToHandle;

		ResourceHandle m_currentMaxHandle = ResourceHandle(0u);
		uint64_t m_frameIndex = 0;
		std::mutex m_mutex;
	};

	class BindlessResourcesManager
	{
	public:
		BindlessResourcesManager();
		~BindlessResourcesManager();

		ResourceHandle RegisterBuffer(WeakPtr<RHI::StorageBuffer> storageBuffer);
		ResourceHandle RegisterImageView(WeakPtr<RHI::ImageView> image);
		ResourceHandle RegisterSamplerState(WeakPtr<RHI::SamplerState> samplerState);

		void UnregisterBuffer(ResourceHandle handle);
		void UnregisterImageView(ResourceHandle handle, RHI::ImageViewType viewType);
		void UnregisterSamplerState(ResourceHandle handle);

		ResourceHandle GetBufferHandle(WeakPtr<RHI::StorageBuffer> storageBuffer);

		void MarkBufferAsDirty(ResourceHandle handle);
		void MarkImageViewAsDirty(ResourceHandle handle, RHI::ImageViewType viewType);
		void MarkSamplerStateAsDirty(ResourceHandle handle);

		void Update();
		void PrepareForRender();

		void PrintResources();

		VT_NODISCARD VT_INLINE RefPtr<RHI::DescriptorTable> GetDescriptorTable() const { return m_bindlessDescriptorTable; }

		VT_NODISCARD VT_INLINE static BindlessResourcesManager& Get() { return *s_instance; }

	private:
		inline static BindlessResourcesManager* s_instance = nullptr;

		ResourceRegistry m_image2DRegistry;
		ResourceRegistry m_image2DArrayRegistry;
		ResourceRegistry m_imageCubeRegistry;

		ResourceRegistry m_bufferRegistry;
		ResourceRegistry m_samplerRegistry;

		RefPtr<RHI::DescriptorTable> m_bindlessDescriptorTable;
	};
}
