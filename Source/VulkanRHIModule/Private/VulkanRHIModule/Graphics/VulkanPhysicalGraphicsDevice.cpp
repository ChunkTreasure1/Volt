#include "vkpch.h"
#include "VulkanRHIModule/Graphics/VulkanPhysicalGraphicsDevice.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Graphics/VulkanGraphicsContext.h"

#include <vulkan/vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		const VkPhysicalDevice FindBestSuitableDevice(VkInstance instance)
		{
			uint32_t deviceCount = 0;
			VT_VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

			if (deviceCount == 0)
			{
				throw std::runtime_error("[PhysicalDevice] Failed to find GPU with Vulkan support!");
			}

			Vector<VkPhysicalDevice> devices{ deviceCount };
			VT_VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

			VkPhysicalDevice nonDiscreteDevice = nullptr;

			VkPhysicalDeviceProperties deviceProperties{};
			for (const auto& device : devices)
			{
				vkGetPhysicalDeviceProperties(device, &deviceProperties);

				constexpr auto VERSION = VK_API_VERSION_1_3;

				if (deviceProperties.apiVersion >= VERSION)
				{
					if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						return device;
					}
				
					nonDiscreteDevice = device;
				}
			}

			if (!nonDiscreteDevice)
			{
				throw std::runtime_error("[PhysicalDevice] Failed to find GPU with Vulkan 1.3 support!");
			}

			return nonDiscreteDevice;
		}

		const PhysicalDeviceQueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			Vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyCount };
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

			PhysicalDeviceQueueFamilyIndices result{};

			for (int32_t i = 0; const auto& queueFamily : queueFamilyProperties)
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && result.graphicsFamilyQueueIndex == -1)
				{
					result.graphicsFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && result.computeFamilyQueueIndex == -1)
				{
					result.computeFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && result.transferFamilyQueueIndex== -1)
				{
					result.transferFamilyQueueIndex = i;
					i++;
					continue;
				}

				if (result.computeFamilyQueueIndex != -1 &&
					result.graphicsFamilyQueueIndex != -1 &&
					result.transferFamilyQueueIndex != -1)
				{
					break;
				}

				i++;
			}

			if (result.computeFamilyQueueIndex == -1)
			{
				result.computeFamilyQueueIndex = result.graphicsFamilyQueueIndex;
			}

			if (result.transferFamilyQueueIndex == -1)
			{
				result.transferFamilyQueueIndex = result.graphicsFamilyQueueIndex;
			}

			return result;
		}
	}

	VulkanPhysicalGraphicsDevice::VulkanPhysicalGraphicsDevice(const PhysicalDeviceCreateInfo& createInfo)
		: m_createInfo(createInfo)
	{
		VkInstance vulkanInstance = GraphicsContext::Get().AsRef<VulkanGraphicsContext>().GetInstance();
		VkPhysicalDevice selectedDevice = Utility::FindBestSuitableDevice(vulkanInstance);

		m_physicalDevice = selectedDevice;
		m_queueFamilyIndices = Utility::FindQueueFamilyIndices(selectedDevice);

		FetchAvailiableExtensions();
		FetchDeviceProperties();
	}

	VulkanPhysicalGraphicsDevice::~VulkanPhysicalGraphicsDevice()
	{
	}

	const int32_t VulkanPhysicalGraphicsDevice::GetMemoryTypeIndex(const uint32_t reqMemoryTypeBits, const uint32_t requiredPropertyFlags)
	{
		for (const auto& memoryType : m_deviceProperties.memoryProperties.memoryTypes)
		{
			const uint32_t memTypeBits = (1 << memoryType.index);
			const bool isRequiredMemoryType = reqMemoryTypeBits & memTypeBits;
		
			const VkMemoryPropertyFlags properties = static_cast<VkMemoryPropertyFlags>(memoryType.propertyFlags);
			const bool hasRequiredProperties = (properties & requiredPropertyFlags) == requiredPropertyFlags;

			if (isRequiredMemoryType && hasRequiredProperties)
			{
				return static_cast<int32_t>(memoryType.index);
			}
		}

		return -1;
	}

	const bool VulkanPhysicalGraphicsDevice::IsExtensionAvailiable(const char* extensionName) const
	{
		for (const auto& ext : m_availiableExtensions)
		{
			if (strcmp(extensionName, ext.extensionName) == 0)
			{
				return true;
			}
		}

		return false;
	}

	const bool VulkanPhysicalGraphicsDevice::AreDescriptorBuffersEnabled() const
	{
		return m_deviceProperties.descriptorBufferProperties.enabled;
	}

	const bool VulkanPhysicalGraphicsDevice::AreMeshShadersEnabled() const
	{
		return m_deviceProperties.meshShaderProperties.enabled;
	}

	void* VulkanPhysicalGraphicsDevice::GetHandleImpl() const
	{
		return m_physicalDevice;
	}

	void VulkanPhysicalGraphicsDevice::FetchMemoryProperties()
	{
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
		{
			auto& memType = m_deviceProperties.memoryProperties.memoryTypes.emplace_back();
			memType.heapIndex = memoryProperties.memoryTypes[i].heapIndex;
			memType.propertyFlags = memoryProperties.memoryTypes[i].propertyFlags;
			memType.index = i;
		}

		for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
		{
			auto& memHeap = m_deviceProperties.memoryProperties.memoryHeaps.emplace_back();
			memHeap.flags = memoryProperties.memoryHeaps[i].flags;
			memHeap.size = memoryProperties.memoryHeaps[i].size;
		}
	}

	void VulkanPhysicalGraphicsDevice::FetchDeviceProperties()
	{
		void* firstChainPtr = nullptr;

		VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};
		descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
		descriptorBufferProperties.pNext = firstChainPtr;
		firstChainPtr = &descriptorBufferProperties;

		VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties{};
		meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
		meshShaderProperties.pNext = firstChainPtr;
		firstChainPtr = &meshShaderProperties;

		VkPhysicalDeviceProperties2	deviceProperties{};
		deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		deviceProperties.pNext = firstChainPtr;

		vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

		m_deviceProperties.deviceName = deviceProperties.properties.deviceName;
		m_deviceProperties.vendor = VendorIDToVendor(deviceProperties.properties.vendorID);

		// Limits
		{
			m_deviceProperties.limits.maxImageDimension1D = deviceProperties.properties.limits.maxImageDimension1D;
			m_deviceProperties.limits.maxImageDimension2D = deviceProperties.properties.limits.maxImageDimension2D;
			m_deviceProperties.limits.maxImageDimension3D = deviceProperties.properties.limits.maxImageDimension3D;
			m_deviceProperties.limits.maxImageDimensionCube = deviceProperties.properties.limits.maxImageDimensionCube;
			m_deviceProperties.limits.maxImageArrayLayers = deviceProperties.properties.limits.maxImageArrayLayers;
			m_deviceProperties.limits.maxTexelBufferElements = deviceProperties.properties.limits.maxTexelBufferElements;
			m_deviceProperties.limits.maxUniformBufferRange = deviceProperties.properties.limits.maxUniformBufferRange;
			m_deviceProperties.limits.maxStorageBufferRange = deviceProperties.properties.limits.maxStorageBufferRange;
			m_deviceProperties.limits.maxPushConstantsSize = deviceProperties.properties.limits.maxPushConstantsSize;
			m_deviceProperties.limits.maxMemoryAllocationCount = deviceProperties.properties.limits.maxMemoryAllocationCount;
			m_deviceProperties.limits.maxSamplerAllocationCount = deviceProperties.properties.limits.maxSamplerAllocationCount;
			m_deviceProperties.limits.bufferImageGranularity = deviceProperties.properties.limits.bufferImageGranularity;
			m_deviceProperties.limits.sparseAddressSpaceSize = deviceProperties.properties.limits.sparseAddressSpaceSize;
			m_deviceProperties.limits.maxBoundDescriptorSets = deviceProperties.properties.limits.maxBoundDescriptorSets;
			m_deviceProperties.limits.maxPerStageDescriptorSamplers = deviceProperties.properties.limits.maxPerStageDescriptorSamplers;
			m_deviceProperties.limits.maxPerStageDescriptorUniformBuffers = deviceProperties.properties.limits.maxPerStageDescriptorUniformBuffers;
			m_deviceProperties.limits.maxPerStageDescriptorStorageBuffers = deviceProperties.properties.limits.maxPerStageDescriptorStorageBuffers;
			m_deviceProperties.limits.maxPerStageDescriptorSampledImages = deviceProperties.properties.limits.maxPerStageDescriptorSampledImages;
			m_deviceProperties.limits.maxPerStageDescriptorStorageImages = deviceProperties.properties.limits.maxPerStageDescriptorStorageImages;
			m_deviceProperties.limits.maxPerStageDescriptorInputAttachments = deviceProperties.properties.limits.maxPerStageDescriptorInputAttachments;
			m_deviceProperties.limits.maxPerStageResources = deviceProperties.properties.limits.maxPerStageResources;
			m_deviceProperties.limits.maxDescriptorSetSamplers = deviceProperties.properties.limits.maxDescriptorSetSamplers;
			m_deviceProperties.limits.maxDescriptorSetUniformBuffers = deviceProperties.properties.limits.maxDescriptorSetUniformBuffers;
			m_deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic = deviceProperties.properties.limits.maxDescriptorSetUniformBuffersDynamic;
			m_deviceProperties.limits.maxDescriptorSetStorageBuffers = deviceProperties.properties.limits.maxDescriptorSetStorageBuffers;
			m_deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic = deviceProperties.properties.limits.maxDescriptorSetStorageBuffersDynamic;
			m_deviceProperties.limits.maxDescriptorSetSampledImages = deviceProperties.properties.limits.maxDescriptorSetSampledImages;
			m_deviceProperties.limits.maxDescriptorSetStorageImages = deviceProperties.properties.limits.maxDescriptorSetStorageImages;
			m_deviceProperties.limits.maxDescriptorSetInputAttachments = deviceProperties.properties.limits.maxDescriptorSetInputAttachments;
			m_deviceProperties.limits.maxVertexInputAttributes = deviceProperties.properties.limits.maxVertexInputAttributes;
			m_deviceProperties.limits.maxVertexInputBindings = deviceProperties.properties.limits.maxVertexInputBindings;
			m_deviceProperties.limits.maxVertexInputAttributeOffset = deviceProperties.properties.limits.maxVertexInputAttributeOffset;
			m_deviceProperties.limits.maxVertexInputBindingStride = deviceProperties.properties.limits.maxVertexInputBindingStride;
			m_deviceProperties.limits.maxVertexOutputComponents = deviceProperties.properties.limits.maxVertexOutputComponents;
			m_deviceProperties.limits.maxTessellationGenerationLevel = deviceProperties.properties.limits.maxTessellationGenerationLevel;
			m_deviceProperties.limits.maxTessellationPatchSize = deviceProperties.properties.limits.maxTessellationPatchSize;
			m_deviceProperties.limits.maxTessellationControlPerVertexInputComponents = deviceProperties.properties.limits.maxTessellationControlPerVertexInputComponents;
			m_deviceProperties.limits.maxTessellationControlPerVertexOutputComponents = deviceProperties.properties.limits.maxTessellationControlPerVertexOutputComponents;
			m_deviceProperties.limits.maxTessellationControlPerPatchOutputComponents = deviceProperties.properties.limits.maxTessellationControlPerPatchOutputComponents;
			m_deviceProperties.limits.maxTessellationControlTotalOutputComponents = deviceProperties.properties.limits.maxTessellationControlTotalOutputComponents;
			m_deviceProperties.limits.maxTessellationEvaluationInputComponents = deviceProperties.properties.limits.maxTessellationEvaluationInputComponents;
			m_deviceProperties.limits.maxTessellationEvaluationOutputComponents = deviceProperties.properties.limits.maxTessellationEvaluationOutputComponents;
			m_deviceProperties.limits.maxGeometryShaderInvocations = deviceProperties.properties.limits.maxGeometryShaderInvocations;
			m_deviceProperties.limits.maxGeometryInputComponents = deviceProperties.properties.limits.maxGeometryInputComponents;
			m_deviceProperties.limits.maxGeometryOutputComponents = deviceProperties.properties.limits.maxGeometryOutputComponents;
			m_deviceProperties.limits.maxGeometryOutputVertices = deviceProperties.properties.limits.maxGeometryOutputVertices;
			m_deviceProperties.limits.maxGeometryTotalOutputComponents = deviceProperties.properties.limits.maxGeometryTotalOutputComponents;
			m_deviceProperties.limits.maxFragmentInputComponents = deviceProperties.properties.limits.maxFragmentInputComponents;
			m_deviceProperties.limits.maxFragmentOutputAttachments = deviceProperties.properties.limits.maxFragmentOutputAttachments;
			m_deviceProperties.limits.maxFragmentDualSrcAttachments = deviceProperties.properties.limits.maxFragmentDualSrcAttachments;
			m_deviceProperties.limits.maxFragmentCombinedOutputResources = deviceProperties.properties.limits.maxFragmentCombinedOutputResources;
			m_deviceProperties.limits.maxComputeSharedMemorySize = deviceProperties.properties.limits.maxComputeSharedMemorySize;
			m_deviceProperties.limits.maxComputeWorkGroupCount[0] = deviceProperties.properties.limits.maxComputeWorkGroupCount[0];
			m_deviceProperties.limits.maxComputeWorkGroupCount[1] = deviceProperties.properties.limits.maxComputeWorkGroupCount[1];
			m_deviceProperties.limits.maxComputeWorkGroupCount[2] = deviceProperties.properties.limits.maxComputeWorkGroupCount[2];
			m_deviceProperties.limits.maxComputeWorkGroupInvocations = deviceProperties.properties.limits.maxComputeWorkGroupInvocations;
			m_deviceProperties.limits.maxComputeWorkGroupSize[0] = deviceProperties.properties.limits.maxComputeWorkGroupSize[0];
			m_deviceProperties.limits.maxComputeWorkGroupSize[1] = deviceProperties.properties.limits.maxComputeWorkGroupSize[1];
			m_deviceProperties.limits.maxComputeWorkGroupSize[2] = deviceProperties.properties.limits.maxComputeWorkGroupSize[2];
			m_deviceProperties.limits.subPixelPrecisionBits = deviceProperties.properties.limits.subPixelPrecisionBits;
			m_deviceProperties.limits.subTexelPrecisionBits = deviceProperties.properties.limits.subTexelPrecisionBits;
			m_deviceProperties.limits.mipmapPrecisionBits = deviceProperties.properties.limits.mipmapPrecisionBits;
			m_deviceProperties.limits.maxDrawIndexedIndexValue = deviceProperties.properties.limits.maxDrawIndexedIndexValue;
			m_deviceProperties.limits.maxDrawIndirectCount = deviceProperties.properties.limits.maxDrawIndirectCount;
			m_deviceProperties.limits.maxSamplerLodBias = deviceProperties.properties.limits.maxSamplerLodBias;
			m_deviceProperties.limits.maxSamplerAnisotropy = deviceProperties.properties.limits.maxSamplerAnisotropy;
			m_deviceProperties.limits.maxViewports = deviceProperties.properties.limits.maxViewports;
			m_deviceProperties.limits.maxViewportDimensions[0] = deviceProperties.properties.limits.maxViewportDimensions[0];
			m_deviceProperties.limits.maxViewportDimensions[1] = deviceProperties.properties.limits.maxViewportDimensions[1];
			m_deviceProperties.limits.viewportBoundsRange[0] = deviceProperties.properties.limits.viewportBoundsRange[0];
			m_deviceProperties.limits.viewportBoundsRange[1] = deviceProperties.properties.limits.viewportBoundsRange[1];
			m_deviceProperties.limits.viewportSubPixelBits = deviceProperties.properties.limits.viewportSubPixelBits;
			m_deviceProperties.limits.minMemoryMapAlignment = deviceProperties.properties.limits.minMemoryMapAlignment;
			m_deviceProperties.limits.minTexelBufferOffsetAlignment = deviceProperties.properties.limits.minTexelBufferOffsetAlignment;
			m_deviceProperties.limits.minUniformBufferOffsetAlignment = deviceProperties.properties.limits.minUniformBufferOffsetAlignment;
			m_deviceProperties.limits.minStorageBufferOffsetAlignment = deviceProperties.properties.limits.minStorageBufferOffsetAlignment;
			m_deviceProperties.limits.minTexelOffset = deviceProperties.properties.limits.minTexelOffset;
			m_deviceProperties.limits.maxTexelOffset = deviceProperties.properties.limits.maxTexelOffset;
			m_deviceProperties.limits.minTexelGatherOffset = deviceProperties.properties.limits.minTexelGatherOffset;
			m_deviceProperties.limits.maxTexelGatherOffset = deviceProperties.properties.limits.maxTexelGatherOffset;
			m_deviceProperties.limits.minInterpolationOffset = deviceProperties.properties.limits.minInterpolationOffset;
			m_deviceProperties.limits.maxInterpolationOffset = deviceProperties.properties.limits.maxInterpolationOffset;
			m_deviceProperties.limits.subPixelInterpolationOffsetBits = deviceProperties.properties.limits.subPixelInterpolationOffsetBits;
			m_deviceProperties.limits.maxFramebufferWidth = deviceProperties.properties.limits.maxFramebufferWidth;
			m_deviceProperties.limits.maxFramebufferHeight = deviceProperties.properties.limits.maxFramebufferHeight;
			m_deviceProperties.limits.maxFramebufferLayers = deviceProperties.properties.limits.maxFramebufferLayers;
			m_deviceProperties.limits.framebufferColorSampleCounts = deviceProperties.properties.limits.framebufferColorSampleCounts;
			m_deviceProperties.limits.framebufferDepthSampleCounts = deviceProperties.properties.limits.framebufferDepthSampleCounts;
			m_deviceProperties.limits.framebufferStencilSampleCounts = deviceProperties.properties.limits.framebufferStencilSampleCounts;
			m_deviceProperties.limits.framebufferNoAttachmentsSampleCounts = deviceProperties.properties.limits.framebufferNoAttachmentsSampleCounts;
			m_deviceProperties.limits.maxColorAttachments = deviceProperties.properties.limits.maxColorAttachments;
			m_deviceProperties.limits.sampledImageColorSampleCounts = deviceProperties.properties.limits.sampledImageColorSampleCounts;
			m_deviceProperties.limits.sampledImageIntegerSampleCounts = deviceProperties.properties.limits.sampledImageIntegerSampleCounts;
			m_deviceProperties.limits.sampledImageDepthSampleCounts = deviceProperties.properties.limits.sampledImageDepthSampleCounts;
			m_deviceProperties.limits.sampledImageStencilSampleCounts = deviceProperties.properties.limits.sampledImageStencilSampleCounts;
			m_deviceProperties.limits.storageImageSampleCounts = deviceProperties.properties.limits.storageImageSampleCounts;
			m_deviceProperties.limits.maxSampleMaskWords = deviceProperties.properties.limits.maxSampleMaskWords;
			m_deviceProperties.limits.timestampComputeAndGraphics = deviceProperties.properties.limits.timestampComputeAndGraphics;
			m_deviceProperties.limits.timestampPeriod = deviceProperties.properties.limits.timestampPeriod;
			m_deviceProperties.limits.maxClipDistances = deviceProperties.properties.limits.maxClipDistances;
			m_deviceProperties.limits.maxCullDistances = deviceProperties.properties.limits.maxCullDistances;
			m_deviceProperties.limits.maxCombinedClipAndCullDistances = deviceProperties.properties.limits.maxCombinedClipAndCullDistances;
			m_deviceProperties.limits.discreteQueuePriorities = deviceProperties.properties.limits.discreteQueuePriorities;
			m_deviceProperties.limits.pointSizeRange[0] = deviceProperties.properties.limits.pointSizeRange[0];
			m_deviceProperties.limits.pointSizeRange[1] = deviceProperties.properties.limits.pointSizeRange[1];
			m_deviceProperties.limits.lineWidthRange[0] = deviceProperties.properties.limits.lineWidthRange[0];
			m_deviceProperties.limits.lineWidthRange[1] = deviceProperties.properties.limits.lineWidthRange[1];
			m_deviceProperties.limits.pointSizeGranularity = deviceProperties.properties.limits.pointSizeGranularity;
			m_deviceProperties.limits.lineWidthGranularity = deviceProperties.properties.limits.lineWidthGranularity;
			m_deviceProperties.limits.strictLines = deviceProperties.properties.limits.strictLines;
			m_deviceProperties.limits.standardSampleLocations = deviceProperties.properties.limits.standardSampleLocations;
			m_deviceProperties.limits.optimalBufferCopyOffsetAlignment = deviceProperties.properties.limits.optimalBufferCopyOffsetAlignment;
			m_deviceProperties.limits.optimalBufferCopyRowPitchAlignment = deviceProperties.properties.limits.optimalBufferCopyRowPitchAlignment;
			m_deviceProperties.limits.nonCoherentAtomSize = deviceProperties.properties.limits.nonCoherentAtomSize;
		}

		// VK_EXT_descriptor_buffer
		{
			m_deviceProperties.descriptorBufferProperties.enabled = false; //IsExtensionAvailiable(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME); // #TODO_Ivar: Temporarily disabled
			m_deviceProperties.descriptorBufferProperties.combinedImageSamplerDescriptorSingleArray = descriptorBufferProperties.combinedImageSamplerDescriptorSingleArray;
			m_deviceProperties.descriptorBufferProperties.bufferlessPushDescriptors = descriptorBufferProperties.bufferlessPushDescriptors;
			m_deviceProperties.descriptorBufferProperties.allowSamplerImageViewPostSubmitCreation = descriptorBufferProperties.allowSamplerImageViewPostSubmitCreation;
			m_deviceProperties.descriptorBufferProperties.descriptorBufferOffsetAlignment = descriptorBufferProperties.descriptorBufferOffsetAlignment;
			m_deviceProperties.descriptorBufferProperties.maxDescriptorBufferBindings = descriptorBufferProperties.maxDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxResourceDescriptorBufferBindings = descriptorBufferProperties.maxResourceDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxSamplerDescriptorBufferBindings = descriptorBufferProperties.maxSamplerDescriptorBufferBindings;
			m_deviceProperties.descriptorBufferProperties.maxEmbeddedImmutableSamplerBindings = descriptorBufferProperties.maxEmbeddedImmutableSamplerBindings;
			m_deviceProperties.descriptorBufferProperties.maxEmbeddedImmutableSamplers = descriptorBufferProperties.maxEmbeddedImmutableSamplers;
			m_deviceProperties.descriptorBufferProperties.bufferCaptureReplayDescriptorDataSize = descriptorBufferProperties.bufferCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.imageCaptureReplayDescriptorDataSize = descriptorBufferProperties.imageCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.imageViewCaptureReplayDescriptorDataSize = descriptorBufferProperties.imageViewCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.samplerCaptureReplayDescriptorDataSize = descriptorBufferProperties.samplerCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.accelerationStructureCaptureReplayDescriptorDataSize = descriptorBufferProperties.accelerationStructureCaptureReplayDescriptorDataSize;
			m_deviceProperties.descriptorBufferProperties.samplerDescriptorSize = descriptorBufferProperties.samplerDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.combinedImageSamplerDescriptorSize = descriptorBufferProperties.combinedImageSamplerDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.sampledImageDescriptorSize = descriptorBufferProperties.sampledImageDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageImageDescriptorSize = descriptorBufferProperties.storageImageDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.uniformTexelBufferDescriptorSize = descriptorBufferProperties.uniformTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustUniformTexelBufferDescriptorSize = descriptorBufferProperties.robustUniformTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageTexelBufferDescriptorSize = descriptorBufferProperties.storageTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustStorageTexelBufferDescriptorSize = descriptorBufferProperties.robustStorageTexelBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.uniformBufferDescriptorSize = descriptorBufferProperties.uniformBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustUniformBufferDescriptorSize = descriptorBufferProperties.robustUniformBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.storageBufferDescriptorSize = descriptorBufferProperties.storageBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.robustStorageBufferDescriptorSize = descriptorBufferProperties.robustStorageBufferDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.inputAttachmentDescriptorSize = descriptorBufferProperties.inputAttachmentDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.accelerationStructureDescriptorSize = descriptorBufferProperties.accelerationStructureDescriptorSize;
			m_deviceProperties.descriptorBufferProperties.maxSamplerDescriptorBufferRange = descriptorBufferProperties.maxSamplerDescriptorBufferRange;
			m_deviceProperties.descriptorBufferProperties.maxResourceDescriptorBufferRange = descriptorBufferProperties.maxResourceDescriptorBufferRange;
			m_deviceProperties.descriptorBufferProperties.samplerDescriptorBufferAddressSpaceSize = descriptorBufferProperties.samplerDescriptorBufferAddressSpaceSize;
			m_deviceProperties.descriptorBufferProperties.resourceDescriptorBufferAddressSpaceSize = descriptorBufferProperties.resourceDescriptorBufferAddressSpaceSize;
			m_deviceProperties.descriptorBufferProperties.descriptorBufferAddressSpaceSize = descriptorBufferProperties.descriptorBufferAddressSpaceSize;
		}

		// VK_EXT_mesh_shader
		{
			m_deviceProperties.meshShaderProperties.enabled = false; // IsExtensionAvailiable(VK_EXT_MESH_SHADER_EXTENSION_NAME);
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupTotalCount = meshShaderProperties.maxTaskWorkGroupTotalCount;
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupCount[0] = meshShaderProperties.maxTaskWorkGroupCount[0];
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupCount[1] = meshShaderProperties.maxTaskWorkGroupCount[1];
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupCount[2] = meshShaderProperties.maxTaskWorkGroupCount[2];
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupInvocations = meshShaderProperties.maxTaskWorkGroupInvocations;
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupSize[0] = meshShaderProperties.maxTaskWorkGroupSize[0];
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupSize[1] = meshShaderProperties.maxTaskWorkGroupSize[1];
			m_deviceProperties.meshShaderProperties.maxTaskWorkGroupSize[2] = meshShaderProperties.maxTaskWorkGroupSize[2];
			m_deviceProperties.meshShaderProperties.maxTaskPayloadSize = meshShaderProperties.maxTaskPayloadSize;
			m_deviceProperties.meshShaderProperties.maxTaskSharedMemorySize = meshShaderProperties.maxTaskSharedMemorySize;
			m_deviceProperties.meshShaderProperties.maxTaskPayloadAndSharedMemorySize = meshShaderProperties.maxTaskPayloadAndSharedMemorySize;
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupTotalCount = meshShaderProperties.maxMeshWorkGroupTotalCount;
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupCount[0] = meshShaderProperties.maxMeshWorkGroupCount[0];
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupCount[1] = meshShaderProperties.maxMeshWorkGroupCount[1];
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupCount[2] = meshShaderProperties.maxMeshWorkGroupCount[2];
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupInvocations = meshShaderProperties.maxMeshWorkGroupInvocations;
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupSize[0] = meshShaderProperties.maxMeshWorkGroupSize[0];
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupSize[1] = meshShaderProperties.maxMeshWorkGroupSize[1];
			m_deviceProperties.meshShaderProperties.maxMeshWorkGroupSize[2] = meshShaderProperties.maxMeshWorkGroupSize[2];
			m_deviceProperties.meshShaderProperties.maxMeshSharedMemorySize = meshShaderProperties.maxMeshSharedMemorySize;
			m_deviceProperties.meshShaderProperties.maxMeshPayloadAndSharedMemorySize = meshShaderProperties.maxMeshPayloadAndSharedMemorySize;
			m_deviceProperties.meshShaderProperties.maxMeshOutputMemorySize = meshShaderProperties.maxMeshOutputMemorySize;
			m_deviceProperties.meshShaderProperties.maxMeshPayloadAndOutputMemorySize = meshShaderProperties.maxMeshPayloadAndOutputMemorySize;
			m_deviceProperties.meshShaderProperties.maxMeshOutputComponents = meshShaderProperties.maxMeshOutputComponents;
			m_deviceProperties.meshShaderProperties.maxMeshOutputVertices = meshShaderProperties.maxMeshOutputVertices;
			m_deviceProperties.meshShaderProperties.maxMeshOutputPrimitives = meshShaderProperties.maxMeshOutputPrimitives;
			m_deviceProperties.meshShaderProperties.maxMeshOutputLayers = meshShaderProperties.maxMeshOutputLayers;
			m_deviceProperties.meshShaderProperties.maxMeshMultiviewViewCount = meshShaderProperties.maxMeshMultiviewViewCount;
			m_deviceProperties.meshShaderProperties.meshOutputPerVertexGranularity = meshShaderProperties.meshOutputPerVertexGranularity;
			m_deviceProperties.meshShaderProperties.meshOutputPerPrimitiveGranularity = meshShaderProperties.meshOutputPerPrimitiveGranularity;
			m_deviceProperties.meshShaderProperties.maxPreferredTaskWorkGroupInvocations = meshShaderProperties.maxPreferredTaskWorkGroupInvocations;
			m_deviceProperties.meshShaderProperties.maxPreferredMeshWorkGroupInvocations = meshShaderProperties.maxPreferredMeshWorkGroupInvocations;
			m_deviceProperties.meshShaderProperties.prefersLocalInvocationVertexOutput = meshShaderProperties.prefersLocalInvocationVertexOutput;
			m_deviceProperties.meshShaderProperties.prefersLocalInvocationPrimitiveOutput = meshShaderProperties.prefersLocalInvocationPrimitiveOutput;
			m_deviceProperties.meshShaderProperties.prefersCompactVertexOutput = meshShaderProperties.prefersCompactVertexOutput;
			m_deviceProperties.meshShaderProperties.prefersCompactPrimitiveOutput = meshShaderProperties.prefersCompactPrimitiveOutput;
		}

		FetchMemoryProperties();
	}

	void VulkanPhysicalGraphicsDevice::FetchAvailiableExtensions()
	{
		uint32_t extCount = 0;
		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr));
		
		m_availiableExtensions.resize(extCount);
		VT_VK_CHECK(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, reinterpret_cast<VkExtensionProperties*>(m_availiableExtensions.data())));
	}
}
