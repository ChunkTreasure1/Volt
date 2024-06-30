#include "dxpch.h"
#include "D3D12ImGuiImplementation.h"

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_glfw.h>

#include "VoltD3D12/Graphics/D3D12GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"
#include <VoltD3D12/Graphics/D3D12Swapchain.h>
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

namespace Volt::RHI
{
	D3D12ImGuiImplementation::D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo)
	{
		m_info = createInfo;
	}

	D3D12ImGuiImplementation::~D3D12ImGuiImplementation()
	{
		m_descriptorHeap = nullptr;
		m_commandBuffer = nullptr;
		ShutdownAPI();
	}

	void D3D12ImGuiImplementation::BeginAPI()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void D3D12ImGuiImplementation::EndAPI()
	{
		m_commandBuffer->Begin();
		auto d3dCommandBuffer = m_commandBuffer->As<D3D12CommandBuffer>();
		auto cmd = d3dCommandBuffer->GetHandle<ID3D12GraphicsCommandList*>();

		auto swapchain = m_info.swapchain->As<D3D12Swapchain>();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = swapchain->GetCurrentImageResource().Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		cmd->ResourceBarrier(1, &barrier);
		const float clearColor[4] = { 0,0,0, 1 };

		const auto rtvPointer = D3D12_CPU_DESCRIPTOR_HANDLE(swapchain->GetCurrentImageResourceView().GetCPUPointer());

		cmd->ClearRenderTargetView(rtvPointer, clearColor, 0, nullptr);
		cmd->OMSetRenderTargets(1, &rtvPointer, false, nullptr);
		cmd->SetDescriptorHeaps(1, m_descriptorHeap->GetHeap().GetAddressOf());

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		cmd->ResourceBarrier(1, &barrier);
		m_commandBuffer->End();
		m_commandBuffer->Execute();
	}

	void D3D12ImGuiImplementation::InitializeAPI(ImGuiContext* context)
	{
		ImGui_ImplGlfw_InitForVulkan(m_info.window, context, true);
		auto device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();

		{
			DescriptorHeapSpecification spec{};
			spec.descriptorType = D3D12DescriptorType::CBV_SRV_UAV;
			spec.maxDescriptorCount = 1000;
			spec.supportsGPUDescriptors = true;

			m_descriptorHeap = CreateScope<D3D12DescriptorHeap>(spec);
		}

		m_fontTexturePointer = m_descriptorHeap->Allocate();

		ImGui_ImplDX12_Init(device, 3,
		DXGI_FORMAT_R8G8B8A8_UNORM, m_descriptorHeap->GetHeap().Get(),
		D3D12_CPU_DESCRIPTOR_HANDLE(m_fontTexturePointer.GetCPUPointer()),
		D3D12_GPU_DESCRIPTOR_HANDLE(m_fontTexturePointer.GetGPUPointer()));

		m_commandBuffer = CommandBuffer::Create(m_info.swapchain);
	}

	void D3D12ImGuiImplementation::ShutdownAPI()
	{
		ImGui_ImplDX12_Shutdown();
	}

	void* D3D12ImGuiImplementation::GetHandleImpl() const
	{
		return nullptr;
	}

	ImTextureID D3D12ImGuiImplementation::GetTextureID(RefPtr<Image2D> image) const
	{
		return nullptr;
	}

	ImFont* D3D12ImGuiImplementation::AddFont(const std::filesystem::path& fontPath, float pixelSize)
	{
		ImGuiIO& io = ImGui::GetIO();
		return io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), pixelSize);
	}
}
