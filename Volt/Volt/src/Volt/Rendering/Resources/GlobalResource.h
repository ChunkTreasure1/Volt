#pragma once

#include "Volt/Rendering/Resources/GlobalResourceManager.h"
#include "Volt/Core/Base.h"

namespace Volt
{
	template<typename T, ResourceSpecialization SPECIALIZATION = ResourceSpecialization::None>
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

		inline static Scope<GlobalResource<T, SPECIALIZATION>> Create(Ref<T> resource)
		{
			return CreateScope<GlobalResource<T, SPECIALIZATION>>(resource);
		}

	private:
		ResourceHandle m_resourceHandle = Resource::Invalid;
		Ref<T> m_resource;
	};
	
	template<typename T, ResourceSpecialization SPECIALIZATION>
	inline GlobalResource<T, SPECIALIZATION>::GlobalResource(Ref<T> resource)
		: m_resource(resource)
	{
		m_resourceHandle = GlobalResourceManager::RegisterResource<T, SPECIALIZATION>(resource);
	}

	template<typename T, ResourceSpecialization SPECIALIZATION>
	inline GlobalResource<T, SPECIALIZATION>::~GlobalResource()
	{
		GlobalResourceManager::UnregisterResource<T, SPECIALIZATION>(m_resourceHandle);
	}

	template<typename T, ResourceSpecialization SPECIALIZATION>
	inline void GlobalResource<T, SPECIALIZATION>::MarkAsDirty() const
	{
		GlobalResourceManager::MarkAsDirty<T, SPECIALIZATION>(m_resourceHandle);
	}
}
