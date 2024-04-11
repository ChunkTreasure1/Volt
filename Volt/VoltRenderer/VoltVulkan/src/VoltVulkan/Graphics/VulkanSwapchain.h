#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/Core/RHICommon.h>
#include <VoltRHI/Graphics/Swapchain.h>

struct VkSwapchainKHR_T;
struct VkRenderPass_T;
struct VkSurfaceKHR_T;

struct VkFence_T;
struct VkSemaphore_T;
struct VkCommandPool_T;
struct VkCommandBuffer_T;

struct VkImage_T;
struct VkImageView_T;
struct VkFramebuffer_T;

struct GLFWwindow;

namespace Volt::RHI
{
	class VulkanSwapchain final : public Swapchain
	{
	public:
		struct SurfaceFormat
		{
			PixelFormat format;
			ColorSpace colorSpace;
		};
		
		inline constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 3;

		VulkanSwapchain(GLFWwindow* glfwWindow);
		~VulkanSwapchain() override;

		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool enableVSync) override;

		const uint32_t GetCurrentFrame() const override;
		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetFramesInFlight() const override;
		const PixelFormat GetFormat() const override;
		Ref<Image2D> GetCurrentImage() const override;

		inline VkRenderPass_T* GetRenderPass() const { return m_renderPass; }
		inline VkCommandBuffer_T* GetCommandBuffer(const uint32_t index) const { return m_perFrameInFlightData.at(index).commandBuffer; }
		inline VkCommandPool_T* GetCommandPool(const uint32_t index) const { return m_perFrameInFlightData.at(index).commandPool; }
		inline VkFence_T* GetFence(const uint32_t index) const { return m_perFrameInFlightData.at(index).fence; }
		inline VkImage_T* GetCurrentVkImage() const { return m_perImageData.at(m_currentImage).image; }
		inline VkImage_T* GetImageAtIndex(const uint32_t index) const { return m_perImageData.at(index).image; }

		VkFramebuffer_T* GetCurrentFramebuffer() const;

	protected:
		void* GetHandleImpl() const override;
	
	private:
		void Invalidate(const uint32_t width, const uint32_t height, bool enableVSync);
		void Release();

		void QuerySwapchainCapabilities();

		void CreateSwapchain(const uint32_t width, const uint32_t height, bool enableVSync);
		void CreateImageViews();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateSyncObjects();
		void CreateCommandPools();
		void CreateCommandBuffers();

		uint32_t m_currentImage = 0;
		uint32_t m_currentFrame = 0;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		bool m_vSyncEnabled = false;
		bool m_swapchainNeedsRebuild = false;

		uint32_t m_totalImageCount = 0;

		struct PerFrameInFlightData
		{
			VkFence_T* fence = nullptr;
			VkSemaphore_T* renderSemaphore = nullptr;
			VkSemaphore_T* presentSemaphore = nullptr;

			VkCommandPool_T* commandPool = nullptr;
			VkCommandBuffer_T* commandBuffer = nullptr;
		};

		struct PerImageData
		{
			VkImage_T* image = nullptr;
			VkImageView_T* imageView = nullptr;
			VkFramebuffer_T* framebuffer = nullptr;

			Ref<Image2D> imageReference;
		};

		struct SwapchainCapabilities
		{
			uint32_t minImageCount = 0;
			uint32_t maxImageCount = 0;

			Extent2D minImageExtent{};
			Extent2D maxImageExtent{};

			std::vector<PresentMode> presentModes{};
			std::vector<SurfaceFormat> surfaceFormats{};
		};

		SwapchainCapabilities m_capabilities{};

		std::vector<PerFrameInFlightData> m_perFrameInFlightData{};
		std::vector<PerImageData> m_perImageData{};

		VkRenderPass_T* m_renderPass = nullptr;
		VkSwapchainKHR_T* m_swapchain = nullptr;
		VkSurfaceKHR_T* m_surface = nullptr;

		PixelFormat m_swapchainFormat = PixelFormat::UNDEFINED;
	};
}
