#include "rhipch.h"
#include "ResourceRegistry.h"

#include "VoltRHI/RHILog.h"

namespace Volt::RHI
{
	ResourceRegistry::ResourceRegistry()
	{
		m_removalQueue.resize(FRAME_COUNT);
	}

	void ResourceRegistry::Update()
	{
		std::scoped_lock lock{ m_mutex };

		m_removalQueue.at(m_frameIndex).Flush();
		m_frameIndex = (m_frameIndex + 1) % FRAME_COUNT;
	}

	void ResourceRegistry::MarkAsDirty(ResourceHandle handle)
	{
		std::scoped_lock lock{ m_mutex };
		auto it = std::find(m_dirtyResources.begin(), m_dirtyResources.end(), handle);
		if (it == m_dirtyResources.end())
		{
			m_dirtyResources.push_back(handle);
		}
	}

	void ResourceRegistry::ClearDirtyResources()
	{
		m_dirtyResources.clear();
	}

	ResourceHandle ResourceRegistry::RegisterResource(WeakPtr<RHI::RHIInterface> resource, RHI::ImageUsage imageUsage)
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
			newHandle = m_currentMaxHandle++;
			registeredResourcePtr = &m_resources.emplace_back();
		}
		else
		{
			registeredResourcePtr = &m_resources.at(newHandle.Get());
		}

		registeredResourcePtr->handle = newHandle;
		registeredResourcePtr->resource = resource;
		registeredResourcePtr->referenceCount = 1;
		registeredResourcePtr->imageUsage = imageUsage;

		m_resourceHashToHandle[resourceHash] = newHandle;
		m_dirtyResources.emplace_back(newHandle);

		return newHandle;
	}

	void ResourceRegistry::UnregisterResource(ResourceHandle handle)
	{
		std::scoped_lock lock{ m_mutex };

		if (handle >= m_resources.size())
		{
			RHILog::LogTagged(LogSeverity::Warning, "[ResourceRegistry]","Resource with handle{0} is not valid!", handle.Get());
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

		auto it = std::find_if(m_dirtyResources.begin(), m_dirtyResources.end(), [handle](const auto& dirtyHandle)
		{
			return handle == dirtyHandle;
		});

		if (it != m_dirtyResources.end())
		{
			m_dirtyResources.erase(it);
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
