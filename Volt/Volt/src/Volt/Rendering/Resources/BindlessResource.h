#pragma once

#include "Volt/Rendering/Resources/BindlessResourcesManager.h"

#include <type_traits>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class ImageView;
		class SamplerState;
	}

	template<typename T>
	concept BindlessResourceType = std::is_same<T, RHI::StorageBuffer>::value || 
								   std::is_same<T, RHI::ImageView>::value || 
								   std::is_same<T, RHI::SamplerState>::value;

	template<BindlessResourceType T>
	class BindlessResource
	{
	public:
		BindlessResource() = default;
		BindlessResource(const BindlessResource&) = delete;
		BindlessResource& operator=(const BindlessResource&) = delete;

		VT_INLINE BindlessResource(Ref<T> resource)
			: m_resource(resource)
		{
			if constexpr (std::is_same<T, RHI::StorageBuffer>::value)
			{
				m_resourceHandle = BindlessResourcesManager::Get().RegisterBuffer(m_resource);
			}
			else if constexpr (std::is_same<T, RHI::ImageView>::value)
			{
				m_resourceHandle = BindlessResourcesManager::Get().RegisterImageView(m_resource);
			}
			else if constexpr (std::is_same<T, RHI::SamplerState>::value)
			{
				m_resourceHandle = BindlessResourcesManager::Get().RegisterSamplerState(m_resource);
			}

			VT_CORE_ASSERT(m_resourceHandle != Resource::Invalid, "Resource type not implemented!");
		}

		~BindlessResource()
		{
			if (IsValid())
			{
				if constexpr (std::is_same<T, RHI::StorageBuffer>::value)
				{
					BindlessResourcesManager::Get().UnregisterBuffer(m_resourceHandle);
				}
				else if constexpr (std::is_same<T, RHI::ImageView>::value)
				{
					BindlessResourcesManager::Get().UnregisterImageView(m_resourceHandle, m_resource->GetViewType());
				}
				else if constexpr (std::is_same<T, RHI::SamplerState>::value)
				{
					BindlessResourcesManager::Get().UnregisterSamplerState(m_resourceHandle);
				}
			}
		}

		void MarkAsDirty()
		{
			if constexpr (std::is_same<T, RHI::StorageBuffer>::value)
			{
				BindlessResourcesManager::Get().MarkBufferAsDirty(m_resourceHandle);
			}
			else if constexpr (std::is_same<T, RHI::ImageView>::value)
			{
				BindlessResourcesManager::Get().MarkImageViewAsDirty(m_resourceHandle, m_resource->GetViewType());
			}
			else if constexpr (std::is_same<T, RHI::SamplerState>::value)
			{
				BindlessResourcesManager::Get().MarkSamplerStateAsDirty(m_resourceHandle);
			}
		}

		template<typename... Args>
		VT_INLINE VT_NODISCARD static Scope<BindlessResource<T>> CreateScope(Args&&... args)
		{
			return ::CreateScope<BindlessResource<T>>(T::Create(args...));
		}

		template<typename... Args>
		VT_INLINE VT_NODISCARD static Ref<BindlessResource<T>> CreateRef(Args&&... args)
		{
			return ::CreateRef<BindlessResource<T>>(T::Create(args...));
		}

		VT_INLINE VT_NODISCARD bool IsValid() const { return m_resource != nullptr; }
		VT_INLINE VT_NODISCARD ResourceHandle GetResourceHandle() const { return m_resourceHandle; }
		VT_INLINE VT_NODISCARD Weak<T> GetResource() const { return m_resource; }

	private:
		Ref<T> m_resource;
		ResourceHandle m_resourceHandle = Resource::Invalid;
	};

	template<typename T>
	using BindlessResourceScope = Scope<BindlessResource<T>>;

	template<typename T>
	using BindlessResourceRef = Ref<BindlessResource<T>>;
}
