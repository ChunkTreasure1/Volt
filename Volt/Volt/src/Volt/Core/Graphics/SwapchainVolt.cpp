#include "vtpch.h"
//#include "SwapchainVolt.h"
//
//#include "Volt/Core/Graphics/GraphicsContextVolt.h"
//#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
//
//#include "Volt/Core/Profiling.h"
//
//#include "Volt/Core/Graphics/VulkanAllocatorVolt.h"
//
//#include <GLFW/glfw3.h>
//
//namespace Volt
//{
//	SwapchainVolt::SwapchainVolt(GLFWwindow* window)
//	{
//		VT_VK_CHECK(glfwCreateWindowSurface(GraphicsContextVolt::Get().GetInstance(), window, nullptr, &mySurface));
//
//		auto physDevice = GraphicsContextVolt::GetPhysicalDevice();
//
//		const auto& queueIndices = physDevice->GetQueueFamilies();
//
//		VkBool32 supportsPresentation;
//		vkGetPhysicalDeviceSurfaceSupportKHR(GraphicsContextVolt::GetPhysicalDevice()->GetHandle(), queueIndices.graphicsFamilyQueueIndex, mySurface, &supportsPresentation);
//		VT_CORE_ASSERT(supportsPresentation == VK_TRUE, "No queue with presentation support found!");
//
//		auto device = GraphicsContextVolt::GetDevice();
//		myDevice = device->GetHandle();
//		myInstance = GraphicsContextVolt::Get().GetInstance();
//
//		Invalidate(myWidth, myHeight, true);
//	}
//	
//	SwapchainVolt::~SwapchainVolt()
//	{
//		Release();
//
//		myInstance = nullptr;
//		myDevice = nullptr;
//	}
//
//	void SwapchainVolt::Release()
//	{
//		if (!mySwapchain)
//		{
//			return;
//		}
//
//		vkDeviceWaitIdle(myDevice);
//
//		for (auto& perFrameData : myFrameInFlightData)
//		{
//			vkDestroyCommandPool(myDevice, perFrameData.commandPool, nullptr);
//
//			vkDestroySemaphore(myDevice, perFrameData.presentSemaphore, nullptr);
//			vkDestroySemaphore(myDevice, perFrameData.renderSemaphore, nullptr);
//
//			vkDestroyFence(myDevice, perFrameData.fence, nullptr);
//		}
//
//		myFrameInFlightData.clear();
//
//		for (auto& perImageData : myImageData)
//		{
//			vkDestroyImageView(myDevice, perImageData.imageView, nullptr);
//			vkDestroyFramebuffer(myDevice, perImageData.framebuffer, nullptr);
//		}
//
//		myImageData.clear();
//
//		vkDestroyRenderPass(myDevice, myRenderPass, nullptr);
//		vkDestroySwapchainKHR(myDevice, mySwapchain, nullptr);
//		vkDestroySurfaceKHR(myInstance, mySurface, nullptr);
//	}
//
//	void SwapchainVolt::BeginFrame()
//	{
//		VT_PROFILE_FUNCTION();
//		auto device = GraphicsContextVolt::GetDevice()->GetHandle();
//
//		auto& frameData = myFrameInFlightData.at(myCurrentFrame);
//
//		VT_VK_CHECK(vkWaitForFences(device, 1, &frameData.fence, VK_TRUE, 1000000000));
//
//		VkResult swapchainStatus = vkAcquireNextImageKHR(device, mySwapchain, 1000000000, frameData.presentSemaphore, nullptr, &myCurrentImage);
//		if (swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR)
//		{
//			Resize(myWidth, myHeight, myVSyncEnabled);
//		}
//		else if (swapchainStatus != VK_SUCCESS && swapchainStatus != VK_SUBOPTIMAL_KHR)
//		{
//			throw std::runtime_error("Failed to acquire swapchain image!");
//		}
//
//		VT_VK_CHECK(vkResetFences(device, 1, &frameData.fence));
//		VT_VK_CHECK(vkResetCommandPool(device, frameData.commandPool, 0));
//		VulkanAllocatorVolt::SetFrameIndex(myCurrentFrame);
//	}
//
//	void SwapchainVolt::Present()
//	{
//		VT_PROFILE_FUNCTION();
//
//		auto& frameData = myFrameInFlightData.at(myCurrentFrame);
//
//		// Queue Submit
//		{
//			VT_PROFILE_SCOPE("Swapchain::QueueSubmit");
//
//			VkSubmitInfo submitInfo{};
//			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//			submitInfo.commandBufferCount = 1;
//			submitInfo.pCommandBuffers = &frameData.commandBuffer;
//
//			submitInfo.signalSemaphoreCount = 1;
//			submitInfo.pSignalSemaphores = &frameData.renderSemaphore;
//
//			submitInfo.waitSemaphoreCount = 1;
//			submitInfo.pWaitSemaphores = &frameData.presentSemaphore;
//
//			const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//			submitInfo.pWaitDstStageMask = &waitStage;
//
//			GraphicsContextVolt::GetDevice()->FlushCommandBuffer(submitInfo, frameData.fence, QueueTypeVolt::Graphics);
//		}
//
//		// Present to screen
//		{
//			VT_PROFILE_SCOPE("Swapchain::Present");
//
//			VkPresentInfoKHR presentInfo{};
//			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//			presentInfo.swapchainCount = 1;
//			presentInfo.pSwapchains = &mySwapchain;
//
//			presentInfo.pWaitSemaphores = &frameData.renderSemaphore;
//			presentInfo.waitSemaphoreCount = 1;
//			presentInfo.pImageIndices = &myCurrentImage;
//
//			OPTICK_GPU_FLIP(&mySwapchain);
//			VkResult presentResult = GraphicsContextVolt::GetDevice()->QueuePresent(presentInfo, QueueTypeVolt::Graphics);
//			if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
//			{
//				Resize(myWidth, myHeight, myVSyncEnabled);
//			}
//			else if (presentResult != VK_SUCCESS)
//			{
//				throw std::runtime_error("Failed to present swapchain image!");
//			}
//		}
//
//		myCurrentFrame = (myCurrentFrame + 1) % myMaxFramesInFlight;
//	}
//
//	void SwapchainVolt::Resize(uint32_t width, uint32_t height, bool useVSync)
//	{
//		if (!mySwapchain)
//		{
//			return;
//		}
//
//		myWidth = width;
//		myWidth = height;
//		myVSyncEnabled = useVSync;
//
//		QuerySwapchainCapabilities();
//
//		GraphicsContextVolt::GetDevice()->WaitForIdle();
//
//		CreateSwapchain(width, height, useVSync);
//		CreateImageViews();
//		CreateFramebuffers();
//	}
//
//	Ref<SwapchainVolt> SwapchainVolt::Create(GLFWwindow* window)
//	{
//		return CreateRef<SwapchainVolt>(window);
//	}
//
//	void SwapchainVolt::Invalidate(uint32_t width, uint32_t height, bool useVSync)
//	{
//		myWidth = width;
//		myWidth = height;
//
//		QuerySwapchainCapabilities();
//
//		CreateSwapchain(width, height, useVSync);
//		CreateImageViews();
//		CreateRenderPass();
//		CreateFramebuffers();
//		CreateSyncObjects();
//		CreateCommandPools();
//		CreateCommandBuffers();
//	}
//
//	void SwapchainVolt::QuerySwapchainCapabilities()
//	{
//		auto physDevice = GraphicsContextVolt::GetPhysicalDevice();
//		
//		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice->GetHandle(), mySurface, &myCapabilities.capabilities);
//
//		uint32_t formatCount = 0;
//		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice->GetHandle(), mySurface, &formatCount, nullptr);
//
//		if (formatCount > 0)
//		{
//			myCapabilities.formats.resize(formatCount);
//			vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice->GetHandle(), mySurface, &formatCount, myCapabilities.formats.data());
//		}
//
//		uint32_t presentModeCount = 0;
//		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice->GetHandle(), mySurface, &presentModeCount, nullptr);
//		
//		if (presentModeCount > 0)
//		{
//			myCapabilities.presentModes.resize(presentModeCount);
//			vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice->GetHandle(), mySurface, &presentModeCount, myCapabilities.presentModes.data());
//		}
//	}
//
//	VkSurfaceFormatKHR SwapchainVolt::ChooseSwapchainFormat()
//	{
//		for (const auto& format : myCapabilities.formats)
//		{
//			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
//			{
//				return format;
//			}
//		}
//
//		return myCapabilities.formats.front();
//	}
//
//	VkPresentModeKHR SwapchainVolt::ChooseSwapchainPresentMode(bool useVSync)
//	{
//		for (const auto& presentMode : myCapabilities.presentModes)
//		{
//			if (useVSync && presentMode == VK_PRESENT_MODE_FIFO_KHR)
//			{
//				return presentMode;
//			}
//
//			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
//			{
//				return presentMode;
//			}
//		}
//
//		return VK_PRESENT_MODE_FIFO_KHR;
//
//	}
//
//	void SwapchainVolt::CreateSwapchain(uint32_t width, uint32_t height, bool useVSync)
//	{
//		const VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainFormat();
//		const VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(useVSync);
//
//		myImageCount = myCapabilities.capabilities.minImageCount + 1;
//		if (myCapabilities.capabilities.maxImageCount > 0 && myImageCount > myCapabilities.capabilities.maxImageCount)
//		{
//			myImageCount = myCapabilities.capabilities.maxImageCount;
//		}
//
//		// Make sure the requested with are 
//		myWidth = std::clamp(width, myCapabilities.capabilities.minImageExtent.width, myCapabilities.capabilities.maxImageExtent.width);
//		myHeight = std::clamp(height, myCapabilities.capabilities.minImageExtent.height, myCapabilities.capabilities.maxImageExtent.height);
//
//		VkSwapchainKHR oldSwapchain = mySwapchain;
//
//		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
//		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//		swapchainCreateInfo.surface = mySurface;
//		swapchainCreateInfo.minImageCount = myImageCount;
//		swapchainCreateInfo.imageFormat = surfaceFormat.format;
//		swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
//		swapchainCreateInfo.imageExtent.width = myWidth;
//		swapchainCreateInfo.imageExtent.height = myHeight;
//		swapchainCreateInfo.imageArrayLayers = 1;
//		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
//		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//		swapchainCreateInfo.presentMode = presentMode;
//		swapchainCreateInfo.clipped = VK_TRUE;
//		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//		swapchainCreateInfo.oldSwapchain = oldSwapchain;
//
//		auto device = GraphicsContextVolt::GetDevice();
//		VT_VK_CHECK(vkCreateSwapchainKHR(device->GetHandle(), &swapchainCreateInfo, nullptr, &mySwapchain));
//
//		if (oldSwapchain != VK_NULL_HANDLE)
//		{
//			vkDestroySwapchainKHR(myDevice, oldSwapchain, nullptr);
//
//			for (auto& imageData : myImageData)
//			{
//				vkDestroyImageView(myDevice, imageData.imageView, nullptr);
//				vkDestroyFramebuffer(myDevice, imageData.framebuffer, nullptr);
//			}
//		}
//
//		VT_VK_CHECK(vkGetSwapchainImagesKHR(device->GetHandle(), mySwapchain, &myImageCount, nullptr));
//		
//		std::vector<VkImage> images{};
//		myImageData.clear();
//		myImageData.resize(myImageCount);
//		images.resize(myImageCount);
//
//		VT_VK_CHECK(vkGetSwapchainImagesKHR(device->GetHandle(), mySwapchain, &myImageCount, images.data()));
//
//		for (size_t i = 0; i < myImageCount; i++)
//		{
//			myImageData.at(i).image = images.at(i);
//		}
//
//		mySwapchainFormat = surfaceFormat.format;
//	}
//
//	void SwapchainVolt::CreateImageViews()
//	{
//		VkImageViewCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		createInfo.format = mySwapchainFormat;
//		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		createInfo.subresourceRange.baseMipLevel = 0;
//		createInfo.subresourceRange.layerCount = 1;
//		createInfo.subresourceRange.baseArrayLayer = 0;
//		createInfo.subresourceRange.levelCount = 1;
//
//		for (auto& imageData : myImageData)
//		{
//			createInfo.image = imageData.image;
//			VT_VK_CHECK(vkCreateImageView(GraphicsContextVolt::GetDevice()->GetHandle(), &createInfo, nullptr, &imageData.imageView));
//		}
//	}
//
//	void SwapchainVolt::CreateRenderPass()
//	{
//		VkAttachmentDescription colorAttachment{};
//		colorAttachment.format = mySwapchainFormat;
//		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//		VkAttachmentReference attachmentRef{};
//		attachmentRef.attachment = 0;
//		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//		VkSubpassDescription subpassDesc{};
//		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//		subpassDesc.colorAttachmentCount = 1;
//		subpassDesc.pColorAttachments = &attachmentRef;
//		subpassDesc.pDepthStencilAttachment = nullptr;
//
//		VkSubpassDependency subpassDepend{};
//		subpassDepend.srcSubpass = VK_SUBPASS_EXTERNAL;
//		subpassDepend.dstSubpass = 0;
//		subpassDepend.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		subpassDepend.srcAccessMask = 0;
//		subpassDepend.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		subpassDepend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//		VkRenderPassCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//		createInfo.attachmentCount = 1;
//		createInfo.pAttachments = &colorAttachment;
//		createInfo.subpassCount = 1;
//		createInfo.pSubpasses = &subpassDesc;
//		createInfo.dependencyCount = 1;
//		createInfo.pDependencies = &subpassDepend;
//
//		VT_VK_CHECK(vkCreateRenderPass(GraphicsContextVolt::GetDevice()->GetHandle(), &createInfo, nullptr, &myRenderPass));
//	}
//
//	void SwapchainVolt::CreateFramebuffers()
//	{
//		VkFramebufferCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		createInfo.renderPass = myRenderPass;
//		createInfo.attachmentCount = 1;
//		createInfo.width = myWidth;
//		createInfo.height = myHeight;
//		createInfo.layers = 1;
//
//		for (auto& imageData : myImageData)
//		{
//			createInfo.pAttachments = &imageData.imageView;
//			VT_VK_CHECK(vkCreateFramebuffer(GraphicsContextVolt::GetDevice()->GetHandle(), &createInfo, nullptr, &imageData.framebuffer));
//		}
//	}
//
//	void SwapchainVolt::CreateSyncObjects()
//	{
//		VkFenceCreateInfo fenceInfo{};
//		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//		auto device = GraphicsContextVolt::GetDevice()->GetHandle();
//
//		myFrameInFlightData.resize(myMaxFramesInFlight);
//
//		for (auto& frameData : myFrameInFlightData)
//		{
//			VT_VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frameData.fence));
//		}
//
//		VkSemaphoreCreateInfo semaphoreInfo{};
//		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//		for (auto& frameData : myFrameInFlightData)
//		{
//			VT_VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameData.presentSemaphore));
//			VT_VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameData.renderSemaphore));
//		}
//	}
//
//	void SwapchainVolt::CreateCommandPools()
//	{
//		const auto queueIndices = GraphicsContextVolt::GetPhysicalDevice()->GetQueueFamilies();
//
//		VkCommandPoolCreateInfo createInfo{};
//		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//		createInfo.queueFamilyIndex = queueIndices.graphicsFamilyQueueIndex;
//		createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
//
//		for (auto& frameData : myFrameInFlightData)
//		{
//			VT_VK_CHECK(vkCreateCommandPool(GraphicsContextVolt::GetDevice()->GetHandle(), &createInfo, nullptr, &frameData.commandPool));
//		}
//	}
//
//	void SwapchainVolt::CreateCommandBuffers()
//	{
//		VkCommandBufferAllocateInfo allocInfo{};
//		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//		allocInfo.commandBufferCount = 1;
//		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//
//		for (auto& frameData : myFrameInFlightData)
//		{
//			allocInfo.commandPool = frameData.commandPool;
//			VT_VK_CHECK(vkAllocateCommandBuffers(GraphicsContextVolt::GetDevice()->GetHandle(), &allocInfo, &frameData.commandBuffer));
//		}
//	}
//}
