#pragma once
//
//#include "Volt/Core/Base.h"
//
//#include <vulkan/vulkan.h>
//
//namespace Volt
//{
//	class PhysicalGraphicsDeviceVolt;
//	class GraphicsDeviceVolt;
//	class Image2D;
//
//	class GraphicsContextVolt
//	{
//	public:
//		GraphicsContextVolt();
//		~GraphicsContextVolt();
//
//		void Initialize();
//		void Shutdown();
//
//		inline const VkInstance GetInstance() const { return myVulkanInstance; }
//
//		static void SetImageName(VkImage image, const std::string& name);
//
//		static GraphicsContextVolt& Get();
//		static Ref<GraphicsDeviceVolt> GetDevice();
//		static Ref<PhysicalGraphicsDeviceVolt> GetPhysicalDevice();
//		static Ref<GraphicsContextVolt> Create();
//
//	private:
//		struct VulkanFunctions
//		{
//			PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectName;
//
//		} myVulkanFunctions;
//
//		void CreateVulkanInstance();
//		void SetupDebugCallback();
//
//		bool CheckValidationLayerSupport();
//		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);
//		std::vector<const char*> GetRequiredExtensions();
//
//		const std::vector<const char*> myValidationLayers = { "VK_LAYER_KHRONOS_validation" };
//
//		Ref<PhysicalGraphicsDeviceVolt> myPhysicalDevice;
//		Ref<GraphicsDeviceVolt> myDevice;
//
//		VkInstance myVulkanInstance;
//		VkDebugUtilsMessengerEXT myDebugMessenger;
//
//		inline static GraphicsContextVolt* s_instance = nullptr;
//	};
//}
