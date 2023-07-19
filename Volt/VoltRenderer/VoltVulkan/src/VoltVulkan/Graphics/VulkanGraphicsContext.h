#pragma once

#include "VoltRHI/Graphics/GraphicsContext.h"

struct VkInstance_T;
struct VkDebugUtilsMessengerEXT_T;

class PhysicalGraphicsDevice;
class GraphicsDevice;

namespace Volt
{
	class VulkanGraphicsContext final : public GraphicsContext
	{
	public:
		VulkanGraphicsContext(const GraphicsContextCreateInfo& createInfo);
		~VulkanGraphicsContext() override;

		inline VkInstance_T* GetInstance() const { return m_instance; }

	protected:
		void* GetHandleImpl() override;

	private:
		void Initialize();
		void Shutdown();

		void CreateInstance();
		void InitializeDebugCallback();

		const bool CheckValidationLayerSupport() const;
		const std::vector<const char*> GetRequiredExtensions() const;

		VkInstance_T* m_instance = nullptr;
		VkDebugUtilsMessengerEXT_T* m_debugMessenger = nullptr;

		GraphicsContextCreateInfo m_createInfo{};
	};
}
