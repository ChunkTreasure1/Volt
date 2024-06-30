#pragma once

#include "VoltD3D12/Common/ComPtr.h"
#include "VoltD3D12/Descriptors/D3D12DescriptorHeap.h"

#include <VoltRHI/ImGui/ImGuiImplementation.h>

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	class CommandBuffer;
	class D3D12ImGuiImplementation final : public ImGuiImplementation
	{
	public:
		D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo);
		~D3D12ImGuiImplementation() override;
		
		ImTextureID GetTextureID(RefPtr<Image2D> image) const override;
		ImFont* AddFont(const std::filesystem::path& fontPath, float pixelSize) override;

	protected:
		void BeginAPI() override;
		void EndAPI() override;

		void InitializeAPI(ImGuiContext* context) override;
		void ShutdownAPI() override;

		void* GetHandleImpl() const override;
	private:
		ImGuiCreateInfo m_info;

		D3D12DescriptorPointer m_fontTexturePointer;

		Scope<D3D12DescriptorHeap> m_descriptorHeap;
		RefPtr<CommandBuffer> m_commandBuffer;
	};
}
