#include "rhipch.h"
#include "RHIModule/Descriptors/ResourceRegistry.h"

namespace Volt::RHI
{
	ResourceRegistry::ResourceRegistry(uint32_t handleSize, uint64_t frameCount)
		: m_handleSize(handleSize), m_framesInFlight(frameCount)
	{
		m_removalQueue.resize(m_framesInFlight);
		m_dirtyResources.resize(m_framesInFlight);
	}

	void ResourceRegistry::Update()
	{
		std::scoped_lock lock{ m_mutex };

		m_removalQueue.at(m_frameIndex).Flush();
		m_frameIndex = (m_frameIndex + 1) % m_framesInFlight;
	}

	void ResourceRegistry::MarkAsDirty(ResourceHandle handle)
	{
		std::scoped_lock lock{ m_mutex };
		for (auto& dirtyResources : m_dirtyResources)
		{
			auto it = std::find(dirtyResources.begin(), dirtyResources.end(), handle);
			if (it == dirtyResources.end())
			{
				dirtyResources.push_back(handle);
			}
		}
	}

	void ResourceRegistry::ClearDirtyResources()
	{
		m_dirtyResources.at(m_frameIndex).clear();
	}

	ResourceHandle ResourceRegistry::RegisterResource(WeakPtr<RHI::RHIInterface> resource, RHI::ImageUsage imageUsage, uint32_t userData)
	{
		std::scoped_lock lock{ m_mutex };

		const size_t resourceHash = resource.GetHash();
		if (m_resourceHashToHandle.contains(resourceHash))
		{
			const ResourceHandle resourceHandle = m_resourceHashToHandle.at(resourceHash);
			m_resources.at(resourceHandle.Get()).referenceCount++;
			return resourceHandle;
		}

		ResourceHandle newHandle = Resource::Invalid;
		if (!m_vacantResourceHandles.empty())
		{
			newHandle = m_vacantResourceHandles.back();
			m_vacantResourceHandles.pop_back();
		}

		RegisteredResource* registeredResourcePtr = nullptr;

		if (newHandle == Resource::Invalid)
		{
			newHandle = m_currentMaxHandle;
			m_currentMaxHandle = m_currentMaxHandle + static_cast<ResourceHandle>(m_handleSize);

			m_resources.resize(m_currentMaxHandle);

			registeredResourcePtr = &m_resources[newHandle];
		}
		else
		{
			registeredResourcePtr = &m_resources.at(newHandle.Get());
		}

		registeredResourcePtr->handle = newHandle;
		registeredResourcePtr->resource = resource;
		registeredResourcePtr->referenceCount = 1;
		registeredResourcePtr->imageUsage = imageUsage;
		registeredResourcePtr->userData = userData;

		m_resourceHashToHandle[resourceHash] = newHandle;
		
		for (auto& dirtyResources : m_dirtyResources)
		{
			dirtyResources.emplace_back(newHandle);
		}

		return newHandle;
	}

	void ResourceRegistry::UnregisterResource(ResourceHandle handle)
	{
		std::scoped_lock lock{ m_mutex };

		if (handle >= m_resources.size())
		{
			VT_LOGC(Warning, LogRHI, "Resource with handle {0} is not valid!", handle.Get());
			return;
		}

		auto& resource = m_resources.at(handle.Get());
		if (resource.referenceCount > 1)
		{
			resource.referenceCount--;
			return;
		}

		resource.referenceCount = 0;
		m_resourceHashToHandle.erase(resource.resource.GetHash());

		for (auto& dirtyResources : m_dirtyResources)
		{
			auto it = std::find_if(dirtyResources.begin(), dirtyResources.end(), [handle](const auto& dirtyHandle)
			{
				return handle == dirtyHandle;
			});

			if (it != dirtyResources.end())
			{
				dirtyResources.erase(it);
			}
		}

		m_removalQueue.at(m_frameIndex).Push([handle, this]()
		{
			m_vacantResourceHandles.emplace_back(handle);
		});
	}

	ResourceHandle ResourceRegistry::GetResourceHandle(WeakPtr<RHI::RHIInterface> resource)
	{
		return m_resourceHashToHandle.at(resource.GetHash());
	}
}
