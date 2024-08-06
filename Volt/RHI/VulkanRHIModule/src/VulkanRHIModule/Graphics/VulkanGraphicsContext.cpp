#include "vkpch.h"
#include "VulkanGraphicsContext.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Common/VulkanFunctions.h"

#include "VulkanRHIModule/Memory/VulkanTransientHeap.h"

#include "VulkanRHIModule/Descriptors/VulkanBindlessDescriptorLayoutManager.h"

#include <RHIModule/Graphics/PhysicalGraphicsDevice.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Memory/Allocator.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <array>

namespace Volt::RHI
{
	static const Vector<VkValidationFeatureEnableEXT> s_enabledValidationFeatures = { /*VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT, /*VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT*/};

	namespace Utility
	{
		inline static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

		inline static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				func(instance, debugMessenger, pAllocator);
			}
		}

		inline static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
		{
			std::string message = pCallbackData->pMessage;
			if (message.find("-06195") != std::string::npos)
			{
				return VK_FALSE;
			}

			switch (messageSeverity)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					VT_LOG(LogVerbosity::Trace, std::string("Validation layer:") + std::string(pCallbackData->pMessage));
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					VT_LOG(LogVerbosity::Info, std::string("Validation layer:") + std::string(pCallbackData->pMessage));
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					VT_LOG(LogVerbosity::Warning, std::string("Validation layer:") + std::string(pCallbackData->pMessage));
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					VT_LOG(LogVerbosity::Error, std::string("Validation layer:") + std::string(pCallbackData->pMessage));
					break;
			}

			return VK_FALSE;
		}

		inline static void PopulateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& outInfo)
		{
			outInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

			VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

#ifdef VT_DEBUG	
			severityFlags |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif

			outInfo.messageSeverity = severityFlags;

			outInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			outInfo.pfnUserCallback = VulkanDebugCallback;
			outInfo.pUserData = nullptr;
		}

		inline static void PopulateValidationFeaturesInfo(VkValidationFeaturesEXT& outInfo, VkDebugUtilsMessengerCreateInfoEXT& debugInfo)
		{
			outInfo.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			outInfo.pNext = &debugInfo;
			outInfo.disabledValidationFeatureCount = 0;
			outInfo.pDisabledValidationFeatures = nullptr;
			outInfo.enabledValidationFeatureCount = static_cast<uint32_t>(s_enabledValidationFeatures.size());
			outInfo.pEnabledValidationFeatures = s_enabledValidationFeatures.data();
		}
	}

	inline static constexpr std::array<const char*, 1> s_validationLayers = { "VK_LAYER_KHRONOS_validation" };

	VulkanGraphicsContext::VulkanGraphicsContext(const GraphicsContextCreateInfo& createInfo)
		: m_createInfo(createInfo)
	{
		Initialize();
	}

	VulkanGraphicsContext::~VulkanGraphicsContext()
	{
		Shutdown();
	}

	RefPtr<Allocator> VulkanGraphicsContext::GetDefaultAllocatorImpl()
	{
		return m_defaultAllocator;
	}

	RefPtr<Allocator> VulkanGraphicsContext::GetTransientAllocatorImpl()
	{
		return m_transientAllocator;
	}

	RefPtr<ResourceStateTracker> VulkanGraphicsContext::GetResourceStateTrackerImpl()
	{
		return m_resourceStateTracker;
	}

	RefPtr<GraphicsDevice> VulkanGraphicsContext::GetGraphicsDevice() const
	{
		return m_graphicsDevice;
	}

	RefPtr<PhysicalGraphicsDevice> VulkanGraphicsContext::GetPhysicalGraphicsDevice() const
	{
		return m_physicalDevice;
	} 

	void* VulkanGraphicsContext::GetHandleImpl() const
	{
		return m_instance;
	}

	void VulkanGraphicsContext::Initialize()
	{
		CreateInstance();

		m_physicalDevice = PhysicalGraphicsDevice::Create(m_createInfo.physicalDeviceInfo);
		
		GraphicsDeviceCreateInfo graphicsDeviceInfo{};
		graphicsDeviceInfo.physicalDevice = m_physicalDevice;
		m_graphicsDevice = GraphicsDevice::Create(graphicsDeviceInfo);
	
		m_resourceStateTracker = RefPtr<ResourceStateTracker>::Create();
		m_defaultAllocator = DefaultAllocator::Create();
		m_transientAllocator = TransientAllocator::Create();

		VulkanBindlessDescriptorLayoutManager::CreateGlobalDescriptorLayout();
	}

	void VulkanGraphicsContext::Shutdown()
	{
		VulkanBindlessDescriptorLayoutManager::DestroyGlobalDescriptorLayout();

		m_defaultAllocator = nullptr;
		m_transientAllocator = nullptr;

		m_graphicsDevice = nullptr;
		m_physicalDevice = nullptr;

#ifdef VT_ENABLE_VALIDATION
		if (m_debugMessenger)
		{
			Utility::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}
#endif

		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanGraphicsContext::CreateInstance()
	{
#ifdef VT_ENABLE_VALIDATION
		const bool validationLayerSupported = CheckValidationLayerSupport();
		if (!validationLayerSupported)
		{
			VT_LOGC(LogVerbosity::Error, LogVulkanRHI, "Validation layers requested but not supported!");
		}
#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Volt";
		appInfo.pEngineName = "Volt";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		const auto requiredExtensions = GetRequiredExtensions();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef VT_ENABLE_VALIDATION
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		Utility::PopulateDebugMessengerInfo(debugCreateInfo);

		VkValidationFeaturesEXT validationFeatures{};
		Utility::PopulateValidationFeaturesInfo(validationFeatures, debugCreateInfo);

		createInfo.pNext = &validationFeatures;
		createInfo.enabledLayerCount = static_cast<uint32_t>(s_validationLayers.size());
		createInfo.ppEnabledLayerNames = s_validationLayers.data();
#else
		createInfo.pNext = nullptr;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
#endif
		
		VT_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));

		if (!m_instance)
		{
			throw std::runtime_error("[GraphicsContext] This device does not support Vulkan!");
			return;
		}

#ifdef VT_ENABLE_VALIDATION
		VT_VK_CHECK(Utility::CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger));
#endif

		FindVulkanFunctions(m_instance);
	}

	const bool VulkanGraphicsContext::CheckValidationLayerSupport() const
	{
		uint32_t layerCount = 0;
		VT_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

		Vector<VkLayerProperties> layerProperties{ layerCount };
		VT_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));

		for (const char* layerName : s_validationLayers)
		{
			bool layerFound = false;
			for (const auto& layer : layerProperties)
			{
				if (strcmp(layerName, layer.layerName) != 0)
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

	const Vector<const char*> VulkanGraphicsContext::GetRequiredExtensions() const
	{
		uint32_t extensionCount = 0;
		const char** extensions = nullptr;

		extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		Vector<const char*> extensionsVector{ extensions, extensions + extensionCount };
		extensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensionsVector;
	}
}
