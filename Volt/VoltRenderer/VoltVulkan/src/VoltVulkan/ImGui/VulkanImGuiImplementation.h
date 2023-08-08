#pragma once

#include <VoltRHI/ImGui/ImGuiImplementation.h>

struct GLFWwindow;

namespace Volt::RHI
{
	class VulkanImGuiImplementation final : public ImGuiImplementation
	{
	public:
		VulkanImGuiImplementation(const ImGuiCreateInfo& createInfo);
		~VulkanImGuiImplementation() override;

		ImTextureID GetTextureID(Ref<Texture2D> image) const override;
		ImTextureID GetTextureID(Ref<Image2D> image) const override;

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
