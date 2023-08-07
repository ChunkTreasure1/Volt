#pragma once

#include "VoltRHI/ImGui/ImGuiImplementation.h"

struct ID3D12DescriptorHeap;

namespace Volt::RHI
{
	class D3D12ImGuiImplementation final : public ImGuiImplementation
	{
	public:
		D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo);

		void BeginAPI() override;
		void EndAPI() override;
		void InitializeAPI() override;
		void ShutdownAPI() override;
	protected:
	private:
		ImGuiCreateInfo m_info;

		ID3D12DescriptorHeap* m_imguiDescriptorHeap;
	};
}
