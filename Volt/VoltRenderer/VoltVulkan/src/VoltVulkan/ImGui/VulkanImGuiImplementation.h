#pragma once

#include <VoltRHI/ImGui/ImGuiImplementation.h>

struct GLFWwindow;

namespace Volt::RHI
{
	class VulkanImGuiImplementation : public ImGuiImplementation
	{
	public:
		VulkanImGuiImplementation(const ImGuiCreateInfo& createInfo);
		~VulkanImGuiImplementation() override;

	protected:
		void BeginAPI() override;
		void EndAPI() override;

		void InitializeAPI() override;
		void ShutdownAPI() override;

	private:
		void InitializeVulkanData();
		void ReleaseVulkanData();
	
		GLFWwindow* m_windowPtr = nullptr;
		Weak<Swapchain> m_swapchain;
	};
}