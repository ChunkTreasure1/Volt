#pragma once
#include <array>

#include "VoltRHI/Graphics/Swapchain.h"

#include "VoltD3D12/Common/D3D12Fence.h"

struct IDXGISwapChain3;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct ID3D12Resource;
struct GLFWwindow;

namespace Volt::RHI
{
	constexpr uint32_t MaxSwapchainImages = 3;

	struct RenderTarget
	{
		ID3D12Resource* resource = nullptr;
		CD3DX12_CPU_DESCRIPTOR_HANDLE* view = {};
		bool hasID = false;
		uint32_t id = 0;
	};


	class D3D12Swapchain final : public Swapchain
	{
	public:
		D3D12Swapchain(GLFWwindow* window);
		~D3D12Swapchain() override;
	
		void* GetHandleImpl() override;
		void BeginFrame() override;
		void Present() override;
		void Resize(const uint32_t width, const uint32_t height, bool enableVSync) override;

		ID3D12Resource* GetResource() { return m_renderTargets[m_currentImageIndex].resource; }

		RenderTarget& GetRenderTarget() { return m_renderTargets[m_currentImageIndex]; }

		VT_NODISCARD const uint32_t GetCurrentFrame() const override;
		VT_NODISCARD const uint32_t GetWidth() const override;
		VT_NODISCARD const uint32_t GetHeight() const override;

	private:
		GLFWwindow* m_windowHandle;
		IDXGISwapChain3* m_swapchain;


		void Build();
		void CleanUp();


		std::array<RenderTarget, MaxSwapchainImages> m_renderTargets = {};
		std::array<D3D12Fence, MaxSwapchainImages> m_fences = {};
		uint32_t m_width;
		uint32_t m_height;
		bool m_enableVsync;

		uint32_t m_currentImageIndex;
	};
}
