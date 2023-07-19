#include "vkpch.h"
#include "VulkanImGuiImplementation.h"

#include "VoltVulkan/Common/VulkanCommon.h"
#include "VoltVulkan/Graphics/VulkanSwapchain.h"
#include "VoltVulkan/Graphics/VulkanGraphicsDevice.h"

#include <VoltRHI/Buffers/CommandBuffer.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/PhysicalGraphicsDevice.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Volt
{
	static Ref<CommandBuffer> s_commandBuffer;
	static VkDescriptorPool s_descriptorPool;

	namespace Utility
	{
		inline static void CheckImGuiVulkanResults(VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				GraphicsContext::Log(Severity::Error, std::format("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__));
				if (result == VK_ERROR_DEVICE_LOST)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(3s);
				}
				assert(false);
			}
		}
	}

	VulkanImGuiImplementation::VulkanImGuiImplementation(const ImGuiCreateInfo& createInfo)
		: m_swapchain(createInfo.swapchain), m_windowPtr(createInfo.window)
	{
	}

	void VulkanImGuiImplementation::BeginAPI()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void VulkanImGuiImplementation::EndAPI()
	{
		s_commandBuffer->Begin();

		//VkCommandBuffer currentCommandBuffer = s_commandBuffer->GetHandle<VkCommandBuffer>();

		// Begin render pass
		{
			//VkClearValue clearValues[2];
			//clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.f } };
			//clearValues[1].depthStencil = { 1.f, 0 };

			//VkRenderPassBeginInfo beginInfo{};
			//beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			//beginInfo.pNext = nullptr;
			//beginInfo.renderPass = m_swapchain.GetRenderPass();
			//beginInfo.renderArea.offset.x = 0;
			//beginInfo.renderArea.offset.y = 0;
			//beginInfo.renderArea.extent.width = width;
			//beginInfo.renderArea.extent.height = height;
			//beginInfo.clearValueCount = 2;
			//beginInfo.pClearValues = clearValues;
			//beginInfo.framebuffer = swapchain.GetCurrentFramebuffer();

			//vkCmdBeginRenderPass(drawCmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
	}

	void VulkanImGuiImplementation::InitializeAPI()
	{
		ImGui_ImplGlfw_InitForVulkan(m_windowPtr, true);
	}

	void VulkanImGuiImplementation::ShutdownAPI()
	{
		ImGui_ImplGlfw_Shutdown();
	}

	void VulkanImGuiImplementation::InitializeVulkanData()
	{
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		auto device = GraphicsContext::GetDevice();

		VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &s_descriptorPool));

		const auto vulkanSwapchain = m_swapchain.lock()->As<VulkanSwapchain>();

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = GraphicsContext::Get().GetHandle<VkInstance>();
		initInfo.PhysicalDevice = GraphicsContext::Get().GetPhysicalDevice()->GetHandle<VkPhysicalDevice>();
		initInfo.Device = device->GetHandle<VkDevice>();
		initInfo.Queue = device->GetDeviceQueue(QueueType::Graphics)->GetHandle<VkQueue>();

		initInfo.DescriptorPool = s_descriptorPool;
		initInfo.ImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
		initInfo.MinImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = Utility::CheckImGuiVulkanResults;

		ImGui_ImplVulkan_Init(&initInfo, vulkanSwapchain->GetRenderPass());

		// Create font
		{
			Ref<CommandBuffer> commandBuffer = CommandBuffer::Create();
			commandBuffer->Begin();
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle<VkCommandBuffer>());
			commandBuffer->End();
			commandBuffer->ExecuteAndWait();
		
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		s_commandBuffer = CommandBuffer::Create(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT, QueueType::Graphics, true);
	}

	void VulkanImGuiImplementation::ReleaseVulkanData()
	{
		auto device = GraphicsContext::GetDevice()->As<VulkanGraphicsDevice>();
		device->WaitForIdle();

		vkDestroyDescriptorPool(device->GetHandle<VkDevice>(), s_descriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}
}
