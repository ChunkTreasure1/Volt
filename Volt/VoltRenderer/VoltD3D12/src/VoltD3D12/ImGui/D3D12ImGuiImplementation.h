#pragma once

#include "VoltRHI/ImGui/ImGuiImplementation.h"

namespace Volt::RHI
{
	class D3D12ImGuiImplementation final : public ImGuiImplementation
	{
	public:

	protected:
		D3D12ImGuiImplementation(const ImGuiCreateInfo& createInfo);

		void BeginAPI() override;
		void EndAPI() override;
		void InitializeAPI() override;
		void ShutdownAPI() override;
	};
}
