#pragma once

#include "Volt/Core/UUID.h"

#include <unordered_map>
#include <vector>

namespace Volt
{
	namespace RHI
	{
		class SamplerState;
		class Image2D;
	}

	typedef uint32_t ResourceHandle;

	template<typename T>
	struct ResourceContainer
	{
		inline const ResourceHandle GetOrAddResource(Weak<T> resource)
		{
			std::scoped_lock lock{ accessMutex };

			if (resourceToHandleMap.contains(resource))
			{
				return resourceToHandleMap.at(resource);
			}

			ResourceHandle handle = static_cast<ResourceHandle>(resources.size());

			if (!availiableHandles.empty())
			{
				handle = availiableHandles.back();
				availiableHandles.pop_back();
			}

			resources.emplace_back(resource);
			resourceToHandleMap[resource] = handle;

			return handle;
		}

		inline const ResourceHandle GetResourceHandle(Weak<T> resource)
		{
			return resourceToHandleMap.at(resource);
		}

		inline void RemoveResource(ResourceHandle resourceHandle)
		{
			std::scoped_lock lock{ accessMutex };

			if (resourceHandle >= static_cast<uint32_t>(resources.size()))
			{
				return;
			}

			for (const auto& [resource, handle] : resourceToHandleMap)
			{
				if (handle == resourceHandle)
				{
					resourceToHandleMap.erase(resource);
					break;
				}
			}

			resources[resourceHandle] = Weak<T>{};
			availiableHandles.emplace_back(resourceHandle);
		}

		inline void RemoveResource(Weak<T> resource)
		{
			std::scoped_lock lock{ accessMutex };

			if (!resourceToHandleMap.contains(resource))
			{
				return;
			}

			const ResourceHandle resourceHandle = resourceToHandleMap.at(resource);

			resources[resourceHandle] = Weak<T>{};
			resourceToHandleMap.erase(resource);
			availiableHandles.emplace_back(resourceHandle);
		}

		inline std::span<const Weak<T>> GetRange()
		{
			return resources;
		}

		std::map<Weak<T>, ResourceHandle> resourceToHandleMap;
		std::vector<ResourceHandle> availiableHandles;

		std::vector<Weak<T>> resources;
		std::mutex accessMutex;
	};

	class GlobalResourceManager
	{
	public:
		template<typename T>
		static const ResourceHandle RegisterResource(Weak<T> resource);

		template<typename T>
		static void UnregisterResource(ResourceHandle handle);

		template<typename T>
		static void UnregisterResource(Weak<T> resource);

		template<typename T>
		static std::span<const Weak<T>> GetResourceRange();

	private:
		template<typename T>
		static ResourceContainer<T>& GetResourceContainer();

		GlobalResourceManager() = delete;
	};

	template<typename T>
	inline const ResourceHandle GlobalResourceManager::RegisterResource(Weak<T> resource)
	{
		auto& container = GetResourceContainer<T>();
		return container.GetOrAddResource(resource);
	}

	template<typename T>
	inline void GlobalResourceManager::UnregisterResource(ResourceHandle handle)
	{
		auto& container = GetResourceContainer<T>();
		container.RemoveResource(handle);
	}

	template<typename T>
	inline void GlobalResourceManager::UnregisterResource(Weak<T> resource)
	{
		auto& container = GetResourceContainer<T>();
		container.RemoveResource(resource);
	}

	template<typename T>
	inline std::span<const Weak<T>> GlobalResourceManager::GetResourceRange()
	{
		auto& container = GetResourceContainer<T>();
		return container.GetRange();
	}

	template<typename T>
	inline ResourceContainer<T>& GlobalResourceManager::GetResourceContainer()
	{
		static ResourceContainer<T> resourceContainer;
		return resourceContainer;
	}
}
