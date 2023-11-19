#pragma once

#include "Volt/Core/UUID.h"
#include "Volt/Utility/UniqueQueue.h"

#include "Volt/RenderingNew/Resources/ResourceHandle.h"

#include <unordered_map>
#include <vector>
#include <span>

namespace Volt
{
	namespace RHI
	{
		class SamplerState;
		class Image2D;
		class DescriptorTable;
	}

	template<typename T>
	struct ResourceContainer
	{
		inline const ResourceHandle GetOrAddResource(Weak<T> resource)
		{
			std::scoped_lock lock{ accessMutex };

			const auto hash = resource.GetHash();

			if (resourceToHandleMap.contains(hash))
			{
				return resourceToHandleMap.at(hash);
			}

			ResourceHandle handle = static_cast<ResourceHandle>(resources.size());
			availiableHandles.TryPop(handle);

			if (static_cast<ResourceHandle>(resources.size()) < handle + 1)
			{
				resources.resize(handle + 1);
			}

			resourceToHandleMap[hash] = handle;
			resources[handle] = resource;
			dirtyResources.emplace_back(resource);

			return handle;
		}

		inline const ResourceHandle GetResourceHandle(Weak<T> resource)
		{
			return resourceToHandleMap.at(resource.GetHash());
		}

		inline void RemoveResource(ResourceHandle resourceHandle)
		{
			std::scoped_lock lock{ accessMutex };

			if (resourceHandle >= static_cast<ResourceHandle>(resources.size()))
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

			auto it = std::find_if(dirtyResources.begin(), dirtyResources.end(), [&](const auto& dirty) 
			{
				if (dirty == resources[resourceHandle])
				{
					return true;
				}

				return false;
			});

			if (it != dirtyResources.end())
			{
				dirtyResources.erase(it);
			}

			resources[resourceHandle] = Weak<T>{};
			availiableHandles.Push(resourceHandle);
		}

		inline void RemoveResource(Weak<T> resource)
		{
			std::scoped_lock lock{ accessMutex };

			const auto hash = resource.GetHash();

			if (!resourceToHandleMap.contains(hash))
			{
				return;
			}

			const ResourceHandle resourceHandle = resourceToHandleMap.at(hash);

			resources[resourceHandle] = Weak<T>{};
			resourceToHandleMap.erase(hash);
			availiableHandles.Push(resourceHandle);
		}

		inline void MarkAsDirty(ResourceHandle handle)
		{
			std::scoped_lock lock{ accessMutex };
			
			if (handle >= static_cast<ResourceHandle>(resources.size()))
			{
				return;
			}

			auto it = std::find_if(dirtyResources.begin(), dirtyResources.end(), [&](const auto& dirty)
			{
				if (dirty == resources[handle])
				{
					return true;
				}

				return false;
			});

			if (it != dirtyResources.end())
			{
				return;
			}

			dirtyResources.emplace_back(resources[handle]); 
		}

		inline std::span<const Weak<T>> GetRange()
		{
			return resources;
		}

		inline std::span<const Weak<T>> GetDirtyRange()
		{
			return dirtyResources;
		}

		inline void ClearDirty()
		{
			dirtyResources.clear();
		}

		std::map<size_t, ResourceHandle> resourceToHandleMap;
		UniqueQueue<ResourceHandle> availiableHandles;

		std::vector<Weak<T>> resources;
		std::vector<Weak<T>> dirtyResources;

		std::mutex accessMutex;
	};

	class GlobalResourceManager
	{
	public:
		static void Initialize();
		static void Shutdown();

		template<typename T>
		static const ResourceHandle RegisterResource(Weak<T> resource);

		template<typename T>
		static void UnregisterResource(ResourceHandle handle);

		template<typename T>
		static void UnregisterResource(Weak<T> resource);

		template<typename T>
		static std::span<const Weak<T>> GetResourceRange();

		template<typename T>
		static ResourceHandle GetResourceHandle(Weak<T> resource);

		template<typename T>
		static void MarkAsDirty(ResourceHandle handle);
 		
		static void Update();
		inline static Ref<RHI::DescriptorTable> GetDescriptorTable() { return s_globalDescriptorTable; }
	private:
		template<typename T>
		static ResourceContainer<T>& GetResourceContainer();

		inline static Ref<RHI::DescriptorTable> s_globalDescriptorTable;

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
	inline ResourceHandle GlobalResourceManager::GetResourceHandle(Weak<T> resource)
	{
		auto& container = GetResourceContainer<T>();
		return container.GetResourceHandle(resource);
	}

	template<typename T>
	inline void GlobalResourceManager::MarkAsDirty(ResourceHandle handle)
	{
		auto& container = GetResourceContainer<T>();
		container.MarkAsDirty(handle);
	}

	template<typename T>
	inline ResourceContainer<T>& GlobalResourceManager::GetResourceContainer()
	{
		static ResourceContainer<T> resourceContainer;
		return resourceContainer;
	}
}
