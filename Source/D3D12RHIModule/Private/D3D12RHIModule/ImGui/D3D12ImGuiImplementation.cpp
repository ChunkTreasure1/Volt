#include "dxpch.h"
#include "D3D12RHIModule/ImGui/D3D12ImGuiImplementation.h"

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_glfw.h>

#include "D3D12RHIModule/Graphics/D3D12GraphicsDevice.h"
#include "D3D12RHIModule/Graphics/D3D12Swapchain.h"
#include "D3D12RHIModule/Buffers/D3D12CommandBuffer.h"
#include "D3D12RHIModule/Images/D3D12ImageView.h"

#include <RHIModule/Utility/ResourceUtility.h>

namespace Volt::RHI
{
	D3D12ImGuiImplementation::D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo)
		: m_framesInFlight(createInfo.swapchain->GetFramesInFlight())
	{
		m_info = createInfo;
		m_descriptorCache.resize(m_framesInFlight);
	}

	D3D12ImGuiImplementation::~D3D12ImGuiImplementation()
	{
		m_descriptorCache.clear();
		m_commandBuffers.clear();
		ShutdownAPI();
	}

	void D3D12ImGuiImplementation::BeginAPI()
	{
		m_currentFrameIndex = (m_currentFrameIndex + 1) % m_framesInFlight;
		for (const auto& descriptor : m_descriptorCache.at(m_currentFrameIndex))
		{
			m_descriptorHeap->Free(descriptor.second);
		}

		m_descriptorCache.at(m_currentFrameIndex).clear();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void D3D12ImGuiImplementation::EndAPI()
	{
		m_commandBuffers.at(m_currentFrameIndex)->Begin();
		auto d3dCommandBuffer = m_commandBuffers.at(m_currentFrameIndex)->As<D3D12CommandBuffer>();
		auto cmd = d3dCommandBuffer->GetHandle<ID3D12GraphicsCommandList*>();

		auto swapchain = m_info.swapchain->As<D3D12Swapchain>();

		{
			RHI::ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Image;

			ResourceUtility::InitializeBarrierSrcFromCurrentState(barrier.imageBarrier(), swapchain->GetCurrentImage());
			barrier.imageBarrier().dstAccess = BarrierAccess::RenderTarget;
			barrier.imageBarrier().dstStage = BarrierStage::RenderTarget;
			barrier.imageBarrier().dstLayout = ImageLayout::RenderTarget;
			barrier.imageBarrier().resource = swapchain->GetCurrentImage();

			m_commandBuffers.at(m_currentFrameIndex)->ResourceBarrier({barrier});
		}

		auto& d3d12View = swapchain->GetCurrentImage()->GetView()->AsRef<D3D12ImageView>();
		const auto rtvPointer = D3D12_CPU_DESCRIPTOR_HANDLE(d3d12View.GetRTVDSVDescriptor().GetCPUPointer());

		const float clearColor[4] = { 0,0,0, 1 };
		cmd->ClearRenderTargetView(rtvPointer, clearColor, 0, nullptr);
		cmd->OMSetRenderTargets(1, &rtvPointer, false, nullptr);
		cmd->SetDescriptorHeaps(1, m_descriptorHeap->GetHeap().GetAddressOf());

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);

		m_commandBuffers.at(m_currentFrameIndex)->End();
		m_commandBuffers.at(m_currentFrameIndex)->Execute();
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

		m_commandBuffers.resize(m_framesInFlight);
		for (uint32_t i = 0; i < m_framesInFlight; i++)
		{
			m_commandBuffers[i] = CommandBuffer::Create();
		}
	}

	void D3D12ImGuiImplementation::ShutdownAPI()
	{
		ImGui_ImplDX12_Shutdown();
	}

	void* D3D12ImGuiImplementation::GetHandleImpl() const
	{
		return nullptr;
	}

	ImTextureID D3D12ImGuiImplementation::GetTextureID(RefPtr<Image> image) const
	{
		const size_t hash = std::hash<void*>()(image->GetHandle<void*>());

		auto& cache = m_descriptorCache.at(m_currentFrameIndex);
		if (cache.contains(hash))
		{
			auto descriptor = m_descriptorCache.at(m_currentFrameIndex).at(hash);
			return reinterpret_cast<ImTextureID>(descriptor.GetGPUPointer());
		}

		D3D12DescriptorPointer resultDescriptor = m_descriptorHeap->Allocate();
		D3D12DescriptorPointer srcDescriptor = image->GetView()->AsRef<D3D12ImageView>().GetSRVDescriptor();

		ID3D12Device2* d3d12Device = GraphicsContext::GetDevice()->GetHandle<ID3D12Device2*>();
		d3d12Device->CopyDescriptorsSimple(1, D3D12_CPU_DESCRIPTOR_HANDLE(resultDescriptor.GetCPUPointer()), D3D12_CPU_DESCRIPTOR_HANDLE(srcDescriptor.GetCPUPointer()), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		cache[hash] = resultDescriptor;
		return reinterpret_cast<ImTextureID>(resultDescriptor.GetGPUPointer());
	}

	ImFont* D3D12ImGuiImplementation::AddFont(const std::filesystem::path& fontPath, float pixelSize)
	{
		ImGuiIO& io = ImGui::GetIO();
		return io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), pixelSize);
	}
}
