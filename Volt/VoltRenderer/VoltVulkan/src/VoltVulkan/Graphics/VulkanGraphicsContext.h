#pragma once

#include "VoltVulkan/Core.h"
#include <VoltRHI/Graphics/GraphicsContext.h>

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;

class PhysicalGraphicsDevice;
class GraphicsDevice;

namespace Volt::RHI
{
	class VulkanGraphicsContext final : public GraphicsContext
	{
	public:
		VulkanGraphicsContext(const GraphicsContextCreateInfo& createInfo);
		~VulkanGraphicsContext() override;

		inline VkInstance_T* GetInstance() const { return m_instance; }

	protected:
		RefPtr<Allocator> GetDefaultAllocatorImpl() override;
		RefPtr<Allocator> GetTransientAllocatorImpl() override;
		RefPtr<ResourceStateTracker> GetResourceStateTrackerImpl() override;

		RefPtr<GraphicsDevice> GetGraphicsDevice() const override;
		RefPtr<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const override;

		void* GetHandleImpl() const override;

	private:
		void Initialize();
		void Shutdown();
		void CreateInstance();

		const bool CheckValidationLayerSupport() const;
		const Vector<const char*> GetRequiredExtensions() const;

		VkInstance_T* m_instance = nullptr;
		VkDebugUtilsMessengerEXT_T* m_debugMessenger = nullptr;

		RefPtr<GraphicsDevice> m_graphicsDevice;
		RefPtr<PhysicalGraphicsDevice> m_physicalDevice;
		RefPtr<ResourceStateTracker> m_resourceStateTracker;

		RefPtr<Allocator> m_defaultAllocator;
		RefPtr<Allocator> m_transientAllocator;

		GraphicsContextCreateInfo m_createInfo{};
	};
}
