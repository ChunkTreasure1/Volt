#pragma once

#include "Volt/Core/Base.h"

#include <vulkan/vulkan.h>

namespace Volt
{
	class PhysicalGraphicsDevice;
	class GraphicsDevice;
	class Image2D;

	class GraphicsContext
	{
	public:
		GraphicsContext();
		~GraphicsContext();

		void Initialize();
		void Shutdown();

		inline const VkInstance GetInstance() const { return myVulkanInstance; }

		static void SetImageName(VkImage image, const std::string& name);

		static GraphicsContext& Get();
		static Ref<GraphicsDevice> GetDevice();
		static Ref<PhysicalGraphicsDevice> GetPhysicalDevice();
		static Ref<GraphicsContext> Create();

	private:
		struct VulkanFunctions
		{
			PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectName;

		} myVulkanFunctions;

		void CreateVulkanInstance();
		void SetupDebugCallback();

		bool CheckValidationLayerSupport();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);
		std::vector<const char*> GetRequiredExtensions();

		const std::vector<const char*> myValidationLayers = { "VK_LAYER_KHRONOS_validation" };

		Ref<PhysicalGraphicsDevice> myPhysicalDevice;
		Ref<GraphicsDevice> myDevice;

		VkInstance myVulkanInstance;
		VkDebugUtilsMessengerEXT myDebugMessenger;

		inline static GraphicsContext* s_instance = nullptr;
	};
}
