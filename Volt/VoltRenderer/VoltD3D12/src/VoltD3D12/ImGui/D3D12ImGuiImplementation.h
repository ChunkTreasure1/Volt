#pragma once

#include "VoltRHI/ImGui/ImGuiImplementation.h"

namespace Volt
{
	class D3D12ImGuiImplementation final : public ImGuiImplementation
	{
	public:
		void BeginAPI() override;
		void EndAPI() override;
		void InitializeAPI() override;
		void ShutdownAPI() override;
	};
}
