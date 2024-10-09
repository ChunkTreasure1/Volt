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

		class Image;
	}

	template<typename T>
	concept BindlessResourceType = std::is_same<T, RHI::StorageBuffer>::value ||
		std::is_same<T, RHI::ImageView>::value ||
		std::is_same<T, RHI::SamplerState>::value ||
		std::is_same<T, RHI::Image>::value;

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

			else if constexpr (std::is_same<T, RHI::Image>::value)
			{
				m_resourceHandle = BindlessResourcesManager::Get().RegisterImageView(m_resource->GetView());
			}

			VT_ASSERT_MSG(m_resourceHandle != Resource::Invalid, "Resource type not implemented!");
		}

		~BindlessResource()
		{
			if (IsValid())
			{
				if constexpr (std::is_same<T, RHI::SamplerState>::value)
				{
					BindlessResourcesManager::Get().UnregisterSamplerState(m_resourceHandle);
				}
				else
				{
					BindlessResourcesManager::Get().UnregisterResource(m_resourceHandle);
				}
			}
		}

		void MarkAsDirty()
		{
			if constexpr (std::is_same<T, RHI::SamplerState>::value)
			{
				BindlessResourcesManager::Get().MarkSamplerStateAsDirty(m_resourceHandle);
			}
			else
			{
				BindlessResourcesManager::Get().MarkResourceAsDirty(m_resourceHandle);
			}
		}

		template<typename... Args>
		VT_NODISCARD VT_INLINE static Scope<BindlessResource<T>> CreateScope(Args&&... args)
		{
			return ::CreateScope<BindlessResource<T>>(T::Create(args...));
		}

		template<typename... Args>
		VT_NODISCARD VT_INLINE static Ref<BindlessResource<T>> CreateRef(Args&&... args)
		{
			return ::CreateRef<BindlessResource<T>>(T::Create(args...));
		}

		VT_NODISCARD VT_INLINE static Scope<BindlessResource<T>> CreateScopeFromResource(RefPtr<T> resource)
		{
			return ::CreateScope<BindlessResource<T>>(resource);
		}

		VT_NODISCARD VT_INLINE static Ref<BindlessResource<T>> CreateRefFromResource(RefPtr<T> resource)
		{
			return ::CreateRef<BindlessResource<T>>(resource);
		}

		VT_NODISCARD VT_INLINE bool IsValid() const { return m_resource != nullptr; }
		VT_NODISCARD VT_INLINE ResourceHandle GetResourceHandle() const { return m_resourceHandle; }
		VT_NODISCARD VT_INLINE RefPtr<T> GetResource() const { return m_resource; }

	private:
		RefPtr<T> m_resource;
		ResourceHandle m_resourceHandle = Resource::Invalid;
	};

	template<typename T>
	using BindlessResourceScope = Scope<BindlessResource<T>>;

	template<typename T>
	using BindlessResourceRef = Ref<BindlessResource<T>>;
}
