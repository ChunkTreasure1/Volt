#include "vtpch.h"
#include "GraphicsContextVolt.h"

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Graphics/VulkanAllocator.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace Volt
{
	namespace Utility
	{
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				func(instance, debugMessenger, pAllocator);
			}
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::string message = pCallbackData->pMessage;
		if (message.find("-06195") != std::string::npos)
		{
			return VK_FALSE;
		}

		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				VT_CORE_TRACE("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				VT_CORE_INFO("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				VT_CORE_WARN("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				VT_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage);
				break;
		}

		return VK_FALSE;
	}

	GraphicsContextVolt::GraphicsContextVolt()
	{
		VT_CORE_ASSERT(s_instance == nullptr, "Graphics context already exists!");
		s_instance = this;

		Initialize();
	}

	GraphicsContextVolt::~GraphicsContextVolt()
	{
		s_instance = nullptr;
		Shutdown();
	}

	void GraphicsContextVolt::Initialize()
	{
		CreateVulkanInstance();
		SetupDebugCallback();

		PhysicalDeviceInfo physDevInfo{};
		physDevInfo.instance = myVulkanInstance;
		physDevInfo.useSeperateComputeQueue = true;
		physDevInfo.useSeperateTransferQueue = true;

		myPhysicalDevice = PhysicalGraphicsDeviceVolt::Create(physDevInfo);

		VkPhysicalDeviceVulkan11Features vulkan11Features{};
		vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		vulkan11Features.pNext = nullptr;
		vulkan11Features.shaderDrawParameters = VK_TRUE;

		VkPhysicalDeviceVulkan12Features vk12Features{};
		vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vk12Features.pNext = &vulkan11Features;
		vk12Features.drawIndirectCount = VK_TRUE;
		vk12Features.samplerFilterMinmax = VK_TRUE;
		vk12Features.hostQueryReset = VK_TRUE;

		vk12Features.runtimeDescriptorArray = VK_TRUE;
		vk12Features.descriptorIndexing = VK_TRUE;
		vk12Features.descriptorBindingPartiallyBound = VK_TRUE;
		vk12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		vk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;

		vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		vk12Features.bufferDeviceAddress = VK_TRUE;
		//vk12Features.shaderFloat16 = VK_TRUE; //#STUPID: Does not work on GTX 1060
		//vk12Features.shaderInt8 = VK_TRUE;

		VkPhysicalDeviceVulkan13Features vk13Features{};
		vk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vk13Features.pNext = &vk12Features;
		vk13Features.synchronization2 = VK_TRUE;
		vk13Features.maintenance4 = VK_TRUE;
		vk13Features.dynamicRendering = VK_TRUE;
		vk13Features.shaderDemoteToHelperInvocation = VK_TRUE;

		VkPhysicalDeviceFeatures2 enabledFeatures{};
		enabledFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		enabledFeatures.features.inheritedQueries = VK_TRUE;
		enabledFeatures.features.multiDrawIndirect = VK_TRUE;
		enabledFeatures.features.geometryShader = VK_TRUE;
		enabledFeatures.features.imageCubeArray = VK_TRUE;
		enabledFeatures.features.samplerAnisotropy = VK_TRUE;
		enabledFeatures.features.pipelineStatisticsQuery = VK_TRUE;
		enabledFeatures.features.fillModeNonSolid = VK_TRUE;
		enabledFeatures.features.wideLines = VK_TRUE;
		enabledFeatures.features.independentBlend = VK_TRUE;
		enabledFeatures.pNext = &vk13Features;

		GraphicsDeviceInfo info{};
		info.physicalDevice = myPhysicalDevice;
		info.requestedFeatures = enabledFeatures;
		info.requestedGraphicsQueues = 2;
		info.requestedTransferQueues = 1;
		info.requestedComputeQueues = 1;

		myDevice = GraphicsDeviceVolt::Create(info);

		VulkanAllocator::Initialize(myDevice);
	}

	void GraphicsContextVolt::Shutdown()
	{
		VulkanAllocator::Shutdown();

		myDevice = nullptr;
		myPhysicalDevice = nullptr;

#ifdef VT_ENABLE_VALIDATION
		Utility::DestroyDebugUtilsMessengerEXT(myVulkanInstance, myDebugMessenger, nullptr);
#endif

		vkDestroyInstance(myVulkanInstance, nullptr);
	}

	void GraphicsContextVolt::SetImageName(VkImage image, const std::string& name)
	{
#ifdef VT_ENABLE_VALIDATION

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		nameInfo.objectHandle = (uint64_t)image;
		nameInfo.pObjectName = name.c_str();

		auto device = GraphicsContextVolt::GetDevice();
		Get().myVulkanFunctions.setDebugUtilsObjectName(device->GetHandle(), &nameInfo);
#endif
	}

	GraphicsContextVolt& GraphicsContextVolt::Get()
	{
		return *s_instance;
	}

	Ref<GraphicsDeviceVolt> GraphicsContextVolt::GetDevice()
	{
		return s_instance->myDevice;
	}

	Ref<PhysicalGraphicsDeviceVolt> GraphicsContextVolt::GetPhysicalDevice()
	{
		return s_instance->myPhysicalDevice;
	}

	Ref<GraphicsContextVolt> GraphicsContextVolt::Create()
	{
		return CreateRef<GraphicsContextVolt>();
	}

	void GraphicsContextVolt::CreateVulkanInstance()
	{
#ifdef VT_ENABLE_VALIDATION
		if (!CheckValidationLayerSupport())
		{
			VT_CORE_ERROR("Validation layers requested, but not available!");
			return;
		}
#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Volt";
		appInfo.pEngineName = "Volt";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		auto extensions = GetRequiredExtensions();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef VT_ENABLE_VALIDATION
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		PopulateDebugMessengerCreateInfo(debugCreateInfo);

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(myValidationLayers.size());
		createInfo.ppEnabledLayerNames = myValidationLayers.data();

#else
		createInfo.pNext = nullptr;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
#endif

		VT_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &myVulkanInstance));
	
#ifdef VT_ENABLE_VALIDATION
		myVulkanFunctions.setDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(myVulkanInstance, "vkSetDebugUtilsObjectNameEXT");
#endif
	}

	void GraphicsContextVolt::SetupDebugCallback()
	{
#ifdef VT_ENABLE_VALIDATION
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		Utility::CreateDebugUtilsMessengerEXT(myVulkanInstance, &createInfo, nullptr, &myDebugMessenger);
#endif
	}

	bool GraphicsContextVolt::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		VT_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

		std::vector<VkLayerProperties> layerProperties(layerCount);
		VT_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));

		for (const char* name : myValidationLayers)
		{
			bool layerFound = false;
			for (const auto& layerProp : layerProperties)
			{
				if (strcmp(name, layerProp.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void GraphicsContextVolt::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo)
	{
		outCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

#ifdef VT_DEBUG	
		severityFlags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif

		outCreateInfo.messageSeverity = severityFlags;

		outCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		outCreateInfo.pfnUserCallback = VulkanDebugCallback;
		outCreateInfo.pUserData = nullptr;
	}

	std::vector<const char*> GraphicsContextVolt::GetRequiredExtensions()
	{
		uint32_t extensionCount = 0;
		const char** extensions;

		extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		std::vector<const char*> extensionsVector{ extensions, extensions + extensionCount };

#ifdef VT_ENABLE_GPU_MARKERS
		extensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensionsVector;
	}
}
