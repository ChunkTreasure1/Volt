#include "vkpch.h"
#include "VulkanSwapchain.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Common/VulkanHelpers.h"

#include "VoltVulkan/Graphics/VulkanGraphicsContext.h"
#include "VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"
#include "VoltVulkan/Graphics/VulkanDeviceQueue.h"

#include <VoltRHI/Core/Profiling.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#ifdef VT_ENABLE_NV_AFTERMATH

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_Defines.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>

#include <VoltRHI/Utility/NsightAftermathHelpers.h>

#endif

namespace Volt::RHI
{
	namespace Utility
	{
		inline static VkSurfaceFormatKHR ChooseSwapchainFormat(const std::span<VulkanSwapchain::SurfaceFormat> swapchainFormats)
		{
			VkSurfaceFormatKHR result{};

			bool foundOptimal = false;

			for (const auto& format : swapchainFormats)
			{
				if (format.format == PixelFormat::R8G8B8A8_UNORM && format.colorSpace == ColorSpace::SRGB_NONLINEAR)
				{
					result.format = Utility::VoltToVulkanFormat(format.format);
					result.colorSpace = Utility::VoltToVulkanColorSpace(format.colorSpace);
					foundOptimal = true;

					break;
				}
			}

			if (!foundOptimal)
			{
				result.format = Utility::VoltToVulkanFormat(swapchainFormats.front().format);
				result.colorSpace = Utility::VoltToVulkanColorSpace(swapchainFormats.front().colorSpace);
			}

			return result;
		}

		inline static VkPresentModeKHR ChooseSwapchainPresentMode(bool useVSync, const std::span<PresentMode> presentModes)
		{
			for (const auto& presentMode : presentModes)
			{
				if (useVSync && presentMode == PresentMode::FIFO)
				{
					return Utility::VoltToVulkanPresentMode(presentMode);
				}

				if (!useVSync && presentMode == PresentMode::Mailbox)
				{
					return Utility::VoltToVulkanPresentMode(presentMode);
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}
	}

	VulkanSwapchain::VulkanSwapchain(GLFWwindow* glfwWindow)
	{
		auto vulkanContext = GraphicsContext::Get().As<VulkanGraphicsContext>();
		auto& vulkanPhysicalDevice = GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>();

		VkInstance instance = vulkanContext->GetHandle<VkInstance>();
		VT_VK_CHECK(glfwCreateWindowSurface(instance, glfwWindow, nullptr, &m_surface));

		const auto& queueFamilies = vulkanPhysicalDevice.GetQueueFamilies();

		VkBool32 supportsPresent = VK_FALSE;
		VT_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(vulkanPhysicalDevice.GetHandle<VkPhysicalDevice>(), queueFamilies.graphicsFamilyQueueIndex, m_surface, &supportsPresent));

		assert(supportsPresent && "Device does not have present support!");

		Invalidate(m_width, m_height, m_vSyncEnabled);
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		Release();
	}

	void VulkanSwapchain::BeginFrame()
	{
		VT_PROFILE_FUNCTION();

		if (m_swapchainNeedsRebuild)
		{
			Resize(m_width, m_height, m_vSyncEnabled);
			m_swapchainNeedsRebuild = false;
		}

		auto device = GraphicsContext::GetDevice();
		auto& frameData = m_perFrameInFlightData.at(m_currentFrame);

		CheckWaitReturnValue(vkWaitForFences(device->GetHandle<VkDevice>(), 1, &frameData.fence, VK_TRUE, 1000000000));
		VkResult swapchainStatus = vkAcquireNextImageKHR(device->GetHandle<VkDevice>(), m_swapchain, 1000000000, frameData.presentSemaphore, nullptr, &m_currentImage);

		CheckWaitReturnValue(vkResetFences(device->GetHandle<VkDevice>(), 1, &frameData.fence));
		CheckWaitReturnValue(vkResetCommandPool(device->GetHandle<VkDevice>(), frameData.commandPool, 0));

		if (swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_swapchainNeedsRebuild = true;
			return;
		}
		else if (swapchainStatus != VK_SUCCESS && swapchainStatus != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swapchain image!");
		}
	}

	void VulkanSwapchain::Present()
	{
		VT_PROFILE_FUNCTION();

		if (m_swapchainNeedsRebuild)
		{
			return;
		}

		auto& frameData = m_perFrameInFlightData.at(m_currentFrame);
		const auto deviceQueue = GraphicsContext::GetDevice()->GetDeviceQueue(QueueType::Graphics);

		VulkanDeviceQueue& vkQueue = deviceQueue->AsRef<VulkanDeviceQueue>();

		// Queue Submit
		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &frameData.commandBuffer;

			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &frameData.renderSemaphore;

			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &frameData.presentSemaphore;

			const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			submitInfo.pWaitDstStageMask = &waitStage;

			vkQueue.AquireLock();
			CheckWaitReturnValue(vkQueueSubmit(deviceQueue->GetHandle<VkQueue>(), 1, &submitInfo, frameData.fence));
			vkQueue.ReleaseLock();
		}

		VT_PROFILE_GPU_FLIP(m_swapchain);

		// Present to screen
		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &m_swapchain;

			presentInfo.pWaitSemaphores = &frameData.renderSemaphore;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pImageIndices = &m_currentImage;

			vkQueue.AquireLock();
			VkResult presentResult = vkQueuePresentKHR(deviceQueue->GetHandle<VkQueue>(), &presentInfo);
			vkQueue.ReleaseLock();

			GetNextFrameIndex();

			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
			{
				m_swapchainNeedsRebuild = true;
				return;
			}
			else if (presentResult != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to present swapchain image!");
			}
		}

	}

	void VulkanSwapchain::Resize(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		if (!m_swapchain)
		{
			return;
		}

		m_width = width;
		m_height = height;
		m_vSyncEnabled = enableVSync;

		QuerySwapchainCapabilities();

		GraphicsContext::GetDevice()->As<VulkanGraphicsDevice>()->WaitForIdle();

		CreateSwapchain(width, height, enableVSync);
		CreateImageViews();
		CreateFramebuffers();
	}

	const uint32_t VulkanSwapchain::GetCurrentFrame() const
	{
		return m_currentFrame;
	}

	const uint32_t VulkanSwapchain::GetWidth() const
	{
		return m_width;
	}

	const uint32_t VulkanSwapchain::GetHeight() const
	{
		return m_height;
	}

	const uint32_t VulkanSwapchain::GetFramesInFlight() const
	{
		return VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
	}

	const PixelFormat VulkanSwapchain::GetFormat() const
	{
		return m_swapchainFormat;
	}

	Ref<Image2D> VulkanSwapchain::GetCurrentImage() const
	{
		const auto& data = m_perImageData.at(GetCurrentFrame());
		return data.imageReference;
	}

	VkFramebuffer_T* VulkanSwapchain::GetCurrentFramebuffer() const
	{
		const auto& data = m_perImageData.at(GetCurrentFrame());
		return data.framebuffer;
	}

	void* VulkanSwapchain::GetHandleImpl() const
	{
		return m_swapchain;
	}

	void VulkanSwapchain::Invalidate(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		m_width = width;
		m_height = height;
		m_vSyncEnabled = enableVSync;

		QuerySwapchainCapabilities();

		CreateSwapchain(width, height, enableVSync);
		CreateImageViews();
		CreateRenderPass();
		CreateFramebuffers();
		CreateSyncObjects();
		CreateCommandPools();
		CreateCommandBuffers();
	}

	void VulkanSwapchain::Release()
	{
		if (!m_swapchain)
		{
			return;
		}

		auto device = GraphicsContext::GetDevice();

		for (auto& perFrameData : m_perFrameInFlightData)
		{
			vkDestroyCommandPool(device->GetHandle<VkDevice>(), perFrameData.commandPool, nullptr);

			vkDestroySemaphore(device->GetHandle<VkDevice>(), perFrameData.presentSemaphore, nullptr);
			vkDestroySemaphore(device->GetHandle<VkDevice>(), perFrameData.renderSemaphore, nullptr);

			vkDestroyFence(device->GetHandle<VkDevice>(), perFrameData.fence, nullptr);
		}

		m_perFrameInFlightData.clear();

		for (auto& perImageData : m_perImageData)
		{
			vkDestroyImageView(device->GetHandle<VkDevice>(), perImageData.imageView, nullptr);
			vkDestroyFramebuffer(device->GetHandle<VkDevice>(), perImageData.framebuffer, nullptr);
		}

		m_perImageData.clear();

		vkDestroyRenderPass(device->GetHandle<VkDevice>(), m_renderPass, nullptr);
		vkDestroySwapchainKHR(device->GetHandle<VkDevice>(), m_swapchain, nullptr);
		vkDestroySurfaceKHR(GraphicsContext::Get().GetHandle<VkInstance>(), m_surface, nullptr);
	}

	void VulkanSwapchain::QuerySwapchainCapabilities()
	{
		auto physicalDevice = GraphicsContext::GetPhysicalDevice();

		VkSurfaceCapabilitiesKHR capabilities{};
		VT_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->GetHandle<VkPhysicalDevice>(), m_surface, &capabilities));

		m_capabilities.minImageCount = capabilities.minImageCount;
		m_capabilities.maxImageCount = capabilities.maxImageCount;

		m_capabilities.minImageExtent.width = capabilities.minImageExtent.width;
		m_capabilities.minImageExtent.height = capabilities.minImageExtent.height;
		m_capabilities.maxImageExtent.width = capabilities.maxImageExtent.width;
		m_capabilities.maxImageExtent.height = capabilities.maxImageExtent.height;

		uint32_t formatCount = 0;
		VT_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->GetHandle<VkPhysicalDevice>(), m_surface, &formatCount, nullptr));

		if (formatCount > 0)
		{
			m_capabilities.surfaceFormats.resize(formatCount);
			VT_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->GetHandle<VkPhysicalDevice>(), m_surface, &formatCount, (VkSurfaceFormatKHR*)m_capabilities.surfaceFormats.data()));
		}

		uint32_t presentModeCount = 0;
		VT_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->GetHandle<VkPhysicalDevice>(), m_surface, &presentModeCount, nullptr));

		if (presentModeCount > 0)
		{
			m_capabilities.presentModes.resize(presentModeCount);
			VT_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->GetHandle<VkPhysicalDevice>(), m_surface, &presentModeCount, (VkPresentModeKHR*)m_capabilities.presentModes.data()));
		}
	}

	void VulkanSwapchain::CreateSwapchain(const uint32_t width, const uint32_t height, bool enableVSync)
	{
		const VkSurfaceFormatKHR surfaceFormat = Utility::ChooseSwapchainFormat(m_capabilities.surfaceFormats);
		const VkPresentModeKHR presentMode = Utility::ChooseSwapchainPresentMode(enableVSync, m_capabilities.presentModes);

		m_totalImageCount = m_capabilities.minImageCount + 1;
		if (m_capabilities.maxImageCount > 0 && m_totalImageCount > m_capabilities.maxImageCount)
		{
			m_totalImageCount = m_capabilities.maxImageCount;
		}

		// Make sure the requested size is within the capabilities of the swapchain
		m_width = std::clamp(width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
		m_height = std::clamp(height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);

		VkSwapchainKHR oldSwapchain = m_swapchain;

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = m_surface;
		swapchainCreateInfo.minImageCount = m_totalImageCount;
		swapchainCreateInfo.imageFormat = surfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent.width = m_width;
		swapchainCreateInfo.imageExtent.height = m_height;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapchainCreateInfo.oldSwapchain = oldSwapchain;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateSwapchainKHR(device->GetHandle<VkDevice>(), &swapchainCreateInfo, nullptr, &m_swapchain));

		if (oldSwapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device->GetHandle<VkDevice>(), oldSwapchain, nullptr);

			for (auto& imageData : m_perImageData)
			{
				vkDestroyImageView(device->GetHandle<VkDevice>(), imageData.imageView, nullptr);
				vkDestroyFramebuffer(device->GetHandle<VkDevice>(), imageData.framebuffer, nullptr);
			}
		}

		VT_VK_CHECK(vkGetSwapchainImagesKHR(device->GetHandle<VkDevice>(), m_swapchain, &m_totalImageCount, nullptr));

		std::vector<VkImage> images{};

		m_perImageData.clear();
		m_perImageData.resize(m_totalImageCount);
		images.resize(m_totalImageCount);

		VT_VK_CHECK(vkGetSwapchainImagesKHR(device->GetHandle<VkDevice>(), m_swapchain, &m_totalImageCount, images.data()));

		for (size_t i = 0; i < m_perImageData.size(); i++)
		{
			m_perImageData[i].image = images.at(i);

			SwapchainImageSpecification spec{};
			spec.swapchain = this;
			spec.imageIndex = static_cast<uint32_t>(i);

			m_perImageData[i].imageReference = Image2D::Create(spec);
		}

		m_swapchainFormat = Utility::VulkanToVoltFormat(surfaceFormat.format);
	}

	void VulkanSwapchain::CreateImageViews()
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = Utility::VoltToVulkanFormat(m_swapchainFormat);
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.levelCount = 1;

		auto device = GraphicsContext::GetDevice();

		for (auto& imageData : m_perImageData)
		{
			createInfo.image = imageData.image;
			VT_VK_CHECK(vkCreateImageView(device->GetHandle<VkDevice>(), &createInfo, nullptr, &imageData.imageView));
		}
	}

	void VulkanSwapchain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = Utility::VoltToVulkanFormat(m_swapchainFormat);
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDesc{};
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &attachmentRef;
		subpassDesc.pDepthStencilAttachment = nullptr;

		VkSubpassDependency subpassDep{};
		subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDep.dstSubpass = 0;
		subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDep.srcAccessMask = 0;
		subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpassDesc;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &subpassDep;

		auto device = GraphicsContext::GetDevice();
		VT_VK_CHECK(vkCreateRenderPass(device->GetHandle<VkDevice>(), &createInfo, nullptr, &m_renderPass));
	}

	void VulkanSwapchain::CreateFramebuffers()
	{
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = m_renderPass;
		createInfo.attachmentCount = 1;
		createInfo.width = m_width;
		createInfo.height = m_height;
		createInfo.layers = 1;

		auto device = GraphicsContext::GetDevice();

		for (auto& imageData : m_perImageData)
		{
			createInfo.pAttachments = &imageData.imageView;
			VT_VK_CHECK(vkCreateFramebuffer(device->GetHandle<VkDevice>(), &createInfo, nullptr, &imageData.framebuffer));
		}
	}

	void VulkanSwapchain::CreateSyncObjects()
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto device = GraphicsContext::GetDevice();

		m_perFrameInFlightData.resize(MAX_FRAMES_IN_FLIGHT);

		for (auto& frameData : m_perFrameInFlightData)
		{
			VT_VK_CHECK(vkCreateFence(device->GetHandle<VkDevice>(), &fenceInfo, nullptr, &frameData.fence));
		}

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (auto& frameData : m_perFrameInFlightData)
		{
			VT_VK_CHECK(vkCreateSemaphore(device->GetHandle<VkDevice>(), &semaphoreInfo, nullptr, &frameData.presentSemaphore));
			VT_VK_CHECK(vkCreateSemaphore(device->GetHandle<VkDevice>(), &semaphoreInfo, nullptr, &frameData.renderSemaphore));
		}
	}

	void VulkanSwapchain::CreateCommandPools()
	{
		const auto queueIndices = GraphicsContext::GetPhysicalDevice()->As<VulkanPhysicalGraphicsDevice>()->GetQueueFamilies();

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueIndices.graphicsFamilyQueueIndex;
		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		auto device = GraphicsContext::GetDevice();
		for (auto& frameData : m_perFrameInFlightData)
		{
			VT_VK_CHECK(vkCreateCommandPool(device->GetHandle<VkDevice>(), &createInfo, nullptr, &frameData.commandPool));
		}
	}

	void VulkanSwapchain::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		auto device = GraphicsContext::GetDevice();
		for (auto& frameData : m_perFrameInFlightData)
		{
			allocInfo.commandPool = frameData.commandPool;
			VT_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle<VkDevice>(), &allocInfo, &frameData.commandBuffer));
		}
	}

	void VulkanSwapchain::GetNextFrameIndex()
	{
		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}
