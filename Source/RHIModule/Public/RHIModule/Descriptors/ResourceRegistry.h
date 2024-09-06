#pragma once

#include "RHIModule/Descriptors/ResourceHandle.h"
#include "RHIModule/Core/RHICommon.h"

#include <CoreUtilities/Containers/FunctionQueue.h>

#include <span>

namespace Volt::RHI
{
	class RHIInterface;

	struct RegisteredResource
	{
		ResourceHandle handle;
		uint32_t referenceCount = 0;
		ImageUsage imageUsage;
		uint32_t userData;

		WeakPtr<RHIInterface> resource;
	};

	class VTRHI_API ResourceRegistry
	{
	public:
		ResourceRegistry(uint32_t handleSize, uint64_t framesInFlight);

		ResourceHandle RegisterResource(WeakPtr<RHIInterface> resource, ImageUsage imageUsage = ImageUsage::None, uint32_t userData = 0);
		void UnregisterResource(ResourceHandle handle);

		ResourceHandle GetResourceHandle(WeakPtr<RHIInterface> resource);

		void Update();
		void MarkAsDirty(ResourceHandle handle);
		void ClearDirtyResources();

		VT_NODISCARD VT_INLINE std::mutex& GetMutex() { return m_mutex; }
		VT_NODISCARD VT_INLINE std::span<const ResourceHandle> GetDirtyResources() const { return m_dirtyResources; }
		VT_NODISCARD VT_INLINE const RegisteredResource& GetResource(ResourceHandle resourceHandle) const { return m_resources.at(resourceHandle); }

	private:
		friend class BindlessResourcesManager;

		Vector<RegisteredResource> m_resources;
		Vector<ResourceHandle> m_vacantResourceHandles;
		Vector<ResourceHandle> m_dirtyResources;
		Vector<FunctionQueue> m_removalQueue;

		std::unordered_map<size_t, ResourceHandle> m_resourceHashToHandle;

		ResourceHandle m_currentMaxHandle = ResourceHandle(0u);

		uint32_t m_handleSize;
		uint64_t m_frameIndex = 0;
		uint64_t m_framesInFlight = 0;
		std::mutex m_mutex;
	};
}
