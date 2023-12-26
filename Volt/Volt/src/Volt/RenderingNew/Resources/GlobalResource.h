#pragma once

#include "Volt/RenderingNew/Resources/GlobalResourceManager.h"
#include "Volt/Core/Base.h"

namespace Volt
{
	template<typename T>
	class GlobalResource
	{
	public:
		GlobalResource(Ref<T> resource);
		~GlobalResource();

		GlobalResource(const GlobalResource&) = delete;
		GlobalResource& operator=(const GlobalResource&) = delete;

		[[nodiscard]] inline const ResourceHandle GetResourceHandle() const { return m_resourceHandle; }
		[[nodiscard]] inline const Weak<T> GetResource() const { return m_resource; }

		void MarkAsDirty() const;

		inline static Scope<GlobalResource<T>> Create(Ref<T> resource)
		{
			return CreateScope<GlobalResource<T>>(resource);
		}

	private:
		ResourceHandle m_resourceHandle = Resource::Invalid;
		Ref<T> m_resource;
	};
	
	template<typename T>
	inline GlobalResource<T>::GlobalResource(Ref<T> resource)
		: m_resource(resource)
	{
		m_resourceHandle = GlobalResourceManager::RegisterResource<T>(resource);
	}

	template<typename T>
	inline GlobalResource<T>::~GlobalResource()
	{
		GlobalResourceManager::UnregisterResource<T>(m_resourceHandle);
	}

	template<typename T>
	inline void GlobalResource<T>::MarkAsDirty() const
	{
		GlobalResourceManager::MarkAsDirty<T>(m_resourceHandle);
	}
}
