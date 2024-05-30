#pragma once

#include "VoltVulkan/Core.h"

#include <VoltRHI/ImGui/ImGuiImplementation.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

#include <CoreUtilities/Pointers/RefPtr.h>

struct GLFWwindow;

struct VkDescriptorPool_T;

namespace Volt::RHI
{
	class VulkanImGuiImplementation final : public ImGuiImplementation
	{
	public:
		VulkanImGuiImplementation(const ImGuiCreateInfo& createInfo);
		~VulkanImGuiImplementation() override;

		ImTextureID GetTextureID(RefPtr<Image2D> image) const override;
		ImFont* AddFont(const std::filesystem::path& fontPath, float pixelSize) override;

	protected:
		void BeginAPI() override;
		void EndAPI() override;

		void InitializeAPI(ImGuiContext* context) override;
		void ShutdownAPI() override;

		void* GetHandleImpl() const override;

	private:
		void InitializeVulkanData();
		void ReleaseVulkanData();
	
		GLFWwindow* m_windowPtr = nullptr;
		WeakPtr<Swapchain> m_swapchain;
		RefPtr<CommandBuffer> m_commandBuffer;
		VkDescriptorPool_T* m_descriptorPool;
	};
}
