#pragma once

#include "D3D12RHIModule/Common/ComPtr.h"
#include "D3D12RHIModule/Descriptors/DescriptorCommon.h"

#include <RHIModule/Graphics/Swapchain.h>
#include <RHIModule/Buffers/CommandBuffer.h>

#include <array>

struct IDXGISwapChain3;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct ID3D12Resource;
struct GLFWwindow;

namespace Volt::RHI
{
	constexpr uint32_t MAX_SWAPCHAIN_IMAGES = 3;

	class D3D12Swapchain final : public Swapchain
	{
	public:
		D3D12Swapchain(GLFWwindow* window);
		~D3D12Swapchain() override;
	
		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool enableVSync) override;

		VT_NODISCARD const uint32_t GetCurrentFrame() const override;
		VT_NODISCARD const uint32_t GetWidth() const override;
		VT_NODISCARD const uint32_t GetHeight() const override;
		VT_NODISCARD const uint32_t GetFramesInFlight() const override;
		VT_NODISCARD RefPtr<Image2D> GetCurrentImage() const override;
		VT_NODISCARD RefPtr<CommandBuffer> GetCommandBuffer() const override;
		VT_NODISCARD const PixelFormat GetFormat() const override;

		VT_NODISCARD ComPtr<ID3D12Resource> GetImageAtIndex(const uint32_t index) const { return m_perImageData.at(index).resource; }
	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const uint32_t width, const uint32_t height, bool enableVSync);
		void Release();

		void CreateSwapchain(const uint32_t width, const uint32_t height);
		void GetSwapchainImages();

		struct PerImageData
		{
			ComPtr<ID3D12Resource> resource = nullptr;
			RefPtr<Image2D> imageReference;
		};

		GLFWwindow* m_windowHandle;
		ComPtr<IDXGISwapChain4> m_swapchain;

		RefPtr<CommandBuffer> m_commandBuffer;

		std::array<PerImageData, MAX_SWAPCHAIN_IMAGES> m_perImageData = {};

		uint32_t m_width = 1280;
		uint32_t m_height = 720;
		bool m_enableVsync = false;
		bool m_supportsTearing = false;

		uint32_t m_currentImageIndex = 0;
	};
}
