#include "vkpch.h"
#include "VulkanRHIModule/ImGui/VulkanImGuiImplementation.h"

#include "VulkanRHIModule/Common/VulkanCommon.h"
#include "VulkanRHIModule/Graphics/VulkanSwapchain.h"
#include "VulkanRHIModule/Graphics/VulkanGraphicsDevice.h"
#include "VulkanRHIModule/Common/VulkanHelpers.h"

#include <RHIModule/Core/Profiling.h>
#include <RHIModule/ImGui/FontAwesome.h>

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/PhysicalGraphicsDevice.h>
#include <RHIModule/Graphics/DeviceQueue.h>

#include <RHIModule/Images/ImageView.h>
#include <RHIModule/Images/Image.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Volt::RHI
{
	namespace Utility
	{
		inline static void CheckImGuiVulkanResults(VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				VT_LOGC(Error, LogVulkanRHI, std::format("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__));
				if (result == VK_ERROR_DEVICE_LOST)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(3s);
				}
				VT_ASSERT(false);
			}
		}
	}

	inline void MergeIconsWithLatestFont(float font_size)
	{
		ImGuiIO& io = ImGui::GetIO();
	
		float baseFontSize = font_size; // 13.0f is the size of the default font. Change to the font size you use.
		float iconFontSize = baseFontSize; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
	
		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { VT_ICON_MIN_FA, VT_ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		io.Fonts->AddFontFromFileTTF("Engine/Fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
	}

	VulkanImGuiImplementation::VulkanImGuiImplementation(const ImGuiCreateInfo& createInfo)
		: m_windowPtr(createInfo.window), m_swapchain(createInfo.swapchain)
	{
	}

	VulkanImGuiImplementation::~VulkanImGuiImplementation()
	{
		ShutdownAPI();
	}

	ImTextureID VulkanImGuiImplementation::GetTextureID(RefPtr<Image> image) const
	{
		ImTextureID id = ImGui_ImplVulkan_AddTexture(nullptr, image->GetView()->GetHandle<VkImageView>(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		return id;
	}

	ImFont* VulkanImGuiImplementation::AddFont(const std::filesystem::path& fontPath, float pixelSize)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFont* newFont = io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), pixelSize);
	
		MergeIconsWithLatestFont(pixelSize);

		// Create font
		{
			RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
			commandBuffer->Begin();
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle<VkCommandBuffer>());
			commandBuffer->End();
			commandBuffer->ExecuteAndWait();

			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		return newFont;
	}

	void VulkanImGuiImplementation::BeginAPI()
	{
		VT_PROFILE_FUNCTION();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void VulkanImGuiImplementation::EndAPI()
	{
		VT_PROFILE_FUNCTION();

		auto swapchainPtr = m_swapchain->As<VulkanSwapchain>();
		auto commandBuffer = swapchainPtr->GetCommandBuffer();

		{
			ResourceBarrierInfo barrier{};
			barrier.type = BarrierType::Image;
			barrier.imageBarrier().srcAccess = BarrierAccess::None;
			barrier.imageBarrier().srcStage = BarrierStage::All;
			barrier.imageBarrier().srcLayout = ImageLayout::Undefined;
			barrier.imageBarrier().dstAccess = BarrierAccess::RenderTarget;
			barrier.imageBarrier().dstStage = BarrierStage::RenderTarget;
			barrier.imageBarrier().dstLayout = ImageLayout::RenderTarget;
			barrier.imageBarrier().resource = swapchainPtr->GetCurrentImage();

			commandBuffer->ResourceBarrier({ barrier });
		}

		AttachmentInfo attachment{};
		attachment.view = swapchainPtr->GetCurrentImage()->GetView();
		attachment.clearMode = ClearMode::Clear;
		attachment.clearColor = { 0.1f, 0.1f, 0.1f, 1.f };

		RenderingInfo renderingInfo{};
		renderingInfo.colorAttachments = { attachment };
		renderingInfo.renderArea.extent.width = swapchainPtr->GetWidth();
		renderingInfo.renderArea.extent.height = swapchainPtr->GetHeight();

		commandBuffer->BeginRendering(renderingInfo);

		Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = static_cast<float>(swapchainPtr->GetHeight());
		viewport.width = static_cast<float>(swapchainPtr->GetWidth());
		viewport.height = -static_cast<float>(swapchainPtr->GetHeight());
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		commandBuffer->SetViewports({ viewport });

		Rect2D scissor{};
		scissor.extent.width = swapchainPtr->GetWidth();
		scissor.extent.height = swapchainPtr->GetHeight();
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		commandBuffer->SetScissors({ scissor });

		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer->GetHandle<VkCommandBuffer>());

		commandBuffer->EndRendering();
	}

	void VulkanImGuiImplementation::InitializeAPI(ImGuiContext* context)
	{
		ImGui::SetCurrentContext(context);

		ImGui_ImplGlfw_InitForVulkan(m_windowPtr, context, true);
		InitializeVulkanData();
	}

	void VulkanImGuiImplementation::ShutdownAPI()
	{
		ReleaseVulkanData();
		ImGui_ImplGlfw_Shutdown();
	}

	void* VulkanImGuiImplementation::GetHandleImpl() const
	{
		return nullptr;
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

		VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle<VkDevice>(), &poolInfo, nullptr, &m_descriptorPool));

		const auto vulkanSwapchain = m_swapchain->As<VulkanSwapchain>();

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = GraphicsContext::Get().GetHandle<VkInstance>();
		initInfo.PhysicalDevice = GraphicsContext::GetPhysicalDevice()->GetHandle<VkPhysicalDevice>();
		initInfo.Device = device->GetHandle<VkDevice>();
		initInfo.Queue = device->GetDeviceQueue(QueueType::Graphics)->GetHandle<VkQueue>();

		initInfo.DescriptorPool = m_descriptorPool;
		initInfo.ImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
		initInfo.MinImageCount = VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = Utility::CheckImGuiVulkanResults;

		ImGui_ImplVulkan_Init(&initInfo, Utility::VoltToVulkanFormat(vulkanSwapchain->GetFormat()));

		// Create font
		{
			RefPtr<CommandBuffer> commandBuffer = CommandBuffer::Create();
			commandBuffer->Begin();
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle<VkCommandBuffer>());
			commandBuffer->End();
			commandBuffer->ExecuteAndWait();
		
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	void VulkanImGuiImplementation::ReleaseVulkanData()
	{
		auto device = GraphicsContext::GetDevice()->As<VulkanGraphicsDevice>();

		m_swapchain->GetCommandBuffer()->WaitForFence();

		vkDestroyDescriptorPool(device->GetHandle<VkDevice>(), m_descriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}
}
