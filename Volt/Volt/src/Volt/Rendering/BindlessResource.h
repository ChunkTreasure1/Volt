#pragma once

#include <RenderCore/Resources/BindlessResourcesManager.h>

#include <type_traits>

namespace Volt
{
	namespace RHI
	{
		class StorageBuffer;
		class ImageView;
		class SamplerState;

		class Image2D;
		class Image3D;
	}

	template<typename T>
	concept BindlessResourceType = std::is_same<T, RHI::StorageBuffer>::value || 
								   std::is_same<T, RHI::ImageView>::value || 
								   std::is_same<T, RHI::SamplerState>::value ||
								   std::is_same<T, RHI::Image3D>::value ||
								   std::is_same<T, RHI::Image2D>::value;

	template<BindlessResourceType T>
	class BindlessResource
	{
	public:
		BindlessResource() = default;
		BindlessResource(const BindlessResource&) = delete;
		BindlessResource& operator=(const BindlessResource&) = delete;

		VT_INLINE BindlessResource(RefPtr<T> resource)
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

			else if constexpr (std::is_same<T, RHI::Image3D>::value || std::is_same<T, RHI::Image2D>::value)
			{
				m_resourceHandle = BindlessResourcesManager::Get().RegisterImageView(m_resource->GetView());
			}

			VT_ASSERT_MSG(m_resourceHandle != Resource::Invalid, "Resource type not implemented!");
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

				else if constexpr (std::is_same<T, RHI::Image3D>::value || std::is_same<T, RHI::Image2D>::value)
				{
					BindlessResourcesManager::Get().UnregisterImageView(m_resourceHandle, m_resource->GetView()->GetViewType());
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

			else if constexpr (std::is_same<T, RHI::Image3D>::value || std::is_same<T, RHI::Image2D>::value)
			{
				BindlessResourcesManager::Get().MarkImageViewAsDirty(m_resourceHandle, m_resource->GetView()->GetViewType());
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
		VT_INLINE VT_NODISCARD RefPtr<T> GetResource() const { return m_resource; }

	private:
		RefPtr<T> m_resource;
		ResourceHandle m_resourceHandle = Resource::Invalid;
	};

	template<typename T>
	using BindlessResourceScope = Scope<BindlessResource<T>>;

	template<typename T>
	using BindlessResourceRef = Ref<BindlessResource<T>>;
}
