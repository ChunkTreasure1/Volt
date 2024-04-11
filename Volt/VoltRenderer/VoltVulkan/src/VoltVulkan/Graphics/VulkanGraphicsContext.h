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
		Allocator& GetDefaultAllocatorImpl() override;
		Ref<Allocator> GetTransientAllocatorImpl() override;
		Ref<GraphicsDevice> GetGraphicsDevice() const override;
		Ref<PhysicalGraphicsDevice> GetPhysicalGraphicsDevice() const override;

		void* GetHandleImpl() const override;

	private:
		void Initialize();
		void Shutdown();
		void CreateInstance();

		const bool CheckValidationLayerSupport() const;
		const std::vector<const char*> GetRequiredExtensions() const;

		VkInstance_T* m_instance = nullptr;
		VkDebugUtilsMessengerEXT_T* m_debugMessenger = nullptr;

		Ref<GraphicsDevice> m_graphicsDevice;
		Ref<PhysicalGraphicsDevice> m_physicalDevice;

		Scope<Allocator> m_defaultAllocator;
		Ref<Allocator> m_transientAllocator;

		GraphicsContextCreateInfo m_createInfo{};
	};
}
