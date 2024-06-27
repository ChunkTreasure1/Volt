#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Descriptors/DescriptorCommon.h"

#include <VoltRHI/Graphics/Swapchain.h>
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
		VT_NODISCARD const PixelFormat GetFormat() const override;

	protected:
		void* GetHandleImpl() const override;

	private:
		void Invalidate(const uint32_t width, const uint32_t height, bool enableVSync);
		void Release();

		void CreateSwapchain(const uint32_t width, const uint32_t height);
		void CreateRTVs();
		void CreateFence();

		uint64_t Signal(uint64_t& fenceValue);
		void WaitForFenceValue(uint64_t fenceValue);

		struct PerImageData
		{
			ComPtr<ID3D12Resource> resource = nullptr;
			D3D12DescriptorPointer descriptorPointer = {};
		};

		GLFWwindow* m_windowHandle;
		ComPtr<IDXGISwapChain4> m_swapchain;

		std::array<PerImageData, MAX_SWAPCHAIN_IMAGES> m_perImageData = {};
		std::array<uint64_t, MAX_SWAPCHAIN_IMAGES> m_perFrameFenceValues{};

		ComPtr<ID3D12Fence> m_fence;
		void* m_fenceEventHandle = nullptr;
		uint64_t m_fenceValue = 0;

		uint32_t m_width = 1280;
		uint32_t m_height = 720;
		bool m_enableVsync = false;
		bool m_supportsTearing = false;

		uint32_t m_currentImageIndex = 0;
	};
}
