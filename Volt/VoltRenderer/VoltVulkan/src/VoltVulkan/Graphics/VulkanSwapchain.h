#pragma once

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

namespace Volt
{
	class VulkanSwapchain final : public Swapchain
	{
	public:
		VulkanSwapchain(GLFWwindow* glfwWindow);
		~VulkanSwapchain() override;

		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool useVSync) override;

		const uint32_t GetCurrentFrame() const override;
		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;

	private:
		void* GetHandleImpl() override;

		inline constexpr static uint32_t MAX_FRAMES_IN_FLIGHT = 3;

		uint32_t m_currentImage = 0;
		uint32_t m_currentFrame = 0;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		bool m_vSyncEnabled = false;

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
		};

		std::vector<PerFrameInFlightData> m_perFrameInFlightData{};
		std::vector<PerImageData> m_perImageData{};

		VkRenderPass_T* m_renderPass = nullptr;
		VkSwapchainKHR_T* m_swapchain = nullptr;
		VkSurfaceKHR_T* m_surface = nullptr;
	};
}
