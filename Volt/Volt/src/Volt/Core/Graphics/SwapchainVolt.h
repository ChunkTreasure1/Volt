#pragma once

#include <vulkan/vulkan.h>

//struct GLFWwindow;
//
//namespace Volt
//{
//	class SwapchainVolt
//	{
//	public:
//		SwapchainVolt(GLFWwindow* window);
//		~SwapchainVolt();
//
//		void Release();
//
//		void BeginFrame();
//		void Present();
//
//		void Resize(uint32_t width, uint32_t height, bool useVSync);
//
//		inline const uint32_t GetCurrentFrame() const { return myCurrentFrame; }
//		inline const uint32_t GetMaxFramesInFlight() const { return myMaxFramesInFlight; }
//		inline const uint32_t GetWidth() const { return myWidth; }
//		inline const uint32_t GetHeight() const { return myHeight; }
//
//		inline VkRenderPass GetRenderPass() const { return myRenderPass; }
//		inline VkFramebuffer GetCurrentFramebuffer() const { return myImageData[myCurrentImage].framebuffer; }
//		inline VkImage GetCurrentImage() const { return myImageData[myCurrentImage].image; }
//		inline VkCommandBuffer GetCurrentCommandBuffer() const { return myFrameInFlightData[myCurrentFrame].commandBuffer; }
//		inline VkCommandBuffer GetCommandBuffer(uint32_t index) const { return myFrameInFlightData[index].commandBuffer; }
//		inline VkCommandPool GetCommandPool(uint32_t index) const { return myFrameInFlightData[index].commandPool; }
//
//		static Ref<SwapchainVolt> Create(GLFWwindow* window);
//
//	private:
//		struct MonitorMode
//		{
//			uint32_t width = 0;
//			uint32_t height = 0;
//			float refreshRate = 0.f;
//		};
//
//		void Invalidate(uint32_t width, uint32_t height, bool useVSync);
//
//		void QuerySwapchainCapabilities();
//		VkSurfaceFormatKHR ChooseSwapchainFormat();
//		VkPresentModeKHR ChooseSwapchainPresentMode(bool useVSync);
//
//		void CreateSwapchain(uint32_t width, uint32_t height, bool useVSync);
//		void CreateImageViews();
//		void CreateRenderPass();
//		void CreateFramebuffers();
//		void CreateSyncObjects();
//		void CreateCommandPools();
//		void CreateCommandBuffers();
//
//		const uint32_t myMaxFramesInFlight = 3;
//
//		uint32_t myCurrentImage = 0;
//		uint32_t myCurrentFrame = 0;
//
//		uint32_t myWidth = 1280;
//		uint32_t myHeight = 720;
//		bool myVSyncEnabled = false;
//
//		uint32_t myImageCount = 0;
//
//		struct PerFrameInFlightData
//		{
//			VkFence fence;
//			VkSemaphore renderSemaphore;
//			VkSemaphore presentSemaphore;
//
//			VkCommandPool commandPool;
//			VkCommandBuffer commandBuffer;
//		};
//
//		struct PerImageData
//		{
//			VkImage image;
//			VkImageView imageView;
//			VkFramebuffer framebuffer;
//		};
//
//		struct SwapchainCapabilities
//		{
//			VkSurfaceCapabilitiesKHR capabilities;
//			std::vector<VkSurfaceFormatKHR> formats;
//			std::vector<VkPresentModeKHR> presentModes;
//		};
//
//		std::vector<PerFrameInFlightData> myFrameInFlightData;
//		std::vector<PerImageData> myImageData;
//
//		VkFormat mySwapchainFormat = VK_FORMAT_UNDEFINED;
//		VkRenderPass myRenderPass = nullptr;
//		VkSwapchainKHR mySwapchain = nullptr;
//		VkSurfaceKHR mySurface = nullptr;
//
//		VkDevice myDevice = nullptr;
//		VkInstance myInstance = nullptr;
//
//		MonitorMode myMonitorMode;
//		SwapchainCapabilities myCapabilities;
//	};
//}
