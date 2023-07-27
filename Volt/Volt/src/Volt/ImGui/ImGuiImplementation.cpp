#include "vtpch.h"
#include "ImGuiImplementation.h"

#include "Volt/Log/Log.h"
#include "Volt/Core/Application.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/SwapchainVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"

#include "Volt/Utility/UIUtility.h"

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <imgui_notify.h>
#include <tahoma.h>
#include "FontAwesome.h"

#include <d3d11.h>

#include <vulkan/vulkan.h>

namespace Volt
{
	static std::vector<VkCommandBuffer> s_imguiCommandBuffers;
	static VkDescriptorPool s_descriptorPool;

	namespace Utility
	{
		inline void CheckImGuiVulkanResults(VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				VT_CORE_ERROR("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__);
				if (result == VK_ERROR_DEVICE_LOST)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(3s);
				}
				VT_CORE_ASSERT(result == VK_SUCCESS, "");
			}
		}
	}

	void MergeIconsWithLatestFont(float font_size)
	{
		ImGuiIO& io = ImGui::GetIO();

		float baseFontSize = font_size; // 13.0f is the size of the default font. Change to the font size you use.
		float iconFontSize = baseFontSize; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, VT_ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;
		io.Fonts->AddFontFromFileTTF("Engine/Fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
	}

	std::filesystem::path GetOrCreateIniPath()
	{
		const std::filesystem::path userIniPath = "User/imgui.ini";
		const std::filesystem::path defaultIniPath = "Editor/imgui.ini";

		if (!std::filesystem::exists(userIniPath))
		{
			VT_CORE_INFO("User ini file not found! Copying default!");

			std::filesystem::create_directories(userIniPath.parent_path());
			if (!std::filesystem::exists(defaultIniPath))
			{
				VT_CORE_ERROR("Unable to find default ini file!");
				return "imgui.ini";
			}
			std::filesystem::copy(defaultIniPath, userIniPath.parent_path());
		}

		return userIniPath;
	}

	ImGuiImplementationVolt::ImGuiImplementationVolt()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImFontConfig fontCfg;
		fontCfg.FontDataOwnedByAtlas = true;
		UI::SetFont(FontType::Regular_16, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 16.f));
		MergeIconsWithLatestFont(16.f);
		
		UI::SetFont(FontType::Regular_17, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 17.f));
		MergeIconsWithLatestFont(17.f);

		UI::SetFont(FontType::Regular_20, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 20.f));
		MergeIconsWithLatestFont(20.f);

		UI::SetFont(FontType::Bold_17, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 17.f));
		MergeIconsWithLatestFont(17.f);

		UI::SetFont(FontType::Bold_16, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 16.f));
		MergeIconsWithLatestFont(16.f);

		UI::SetFont(FontType::Bold_90, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 90.f));
		MergeIconsWithLatestFont(90.f);

		UI::SetFont(FontType::Bold_20, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 20.f));
		MergeIconsWithLatestFont(20.f);

		UI::SetFont(FontType::Bold_12, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Bold.ttf", 12.f));
		MergeIconsWithLatestFont(12.f);

		UI::SetFont(FontType::Regular_12, io.Fonts->AddFontFromFileTTF("Engine/Fonts/Inter/inter-Regular.ttf", 12.f));
		MergeIconsWithLatestFont(12.f);

		fontCfg.FontDataOwnedByAtlas = false;
		io.Fonts->AddFontFromMemoryTTF((void*)tahoma, sizeof(tahoma), 17.f, &fontCfg);
		ImGui::MergeIconsWithLatestFont(17.f, false);

		io.IniFilename = nullptr;

		const std::filesystem::path iniPath = GetOrCreateIniPath();
		ImGui::LoadIniSettingsFromDisk(iniPath.string().c_str());

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.f;
		}

		style.Colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.000f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);

		style.Colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);

		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);

		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);

		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Button] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);


		style.Colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);

		style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabTop] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);

		style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.4f, 0.67f, 1.000f, 0.781f);
		style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);

		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);

		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.GrabMinSize = 7.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 12.0f;
		style.ScrollbarSize = 13.0f;
		style.TabBorderSize = 0.0f;
		style.TabRounding = 0.0f;
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 2.f;

		Application& app = Application::Get();
		myWindowPtr = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImGui_ImplGlfw_InitForVulkan(myWindowPtr, true);
		InitializeVulkanData();
	}

	ImGuiImplementationVolt::~ImGuiImplementationVolt()
	{
		ReleaseVulkanData();
		ImGui_ImplGlfw_Shutdown();

		const std::filesystem::path iniPath = GetOrCreateIniPath();
		ImGui::SaveIniSettingsToDisk(iniPath.string().c_str());
		ImGui::DestroyContext();
	}

	void ImGuiImplementationVolt::Begin()
	{
		VT_PROFILE_FUNCTION();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiImplementationVolt::End()
	{
		VT_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
		ImGui::RenderNotifications(); // <-- Here we render all notifications
		ImGui::PopStyleVar(1); // Don't forget to Pop()
		ImGui::PopStyleColor(1);

		//Rendering
		ImGui::Render();

		//auto& swapchain = Application::Get().GetWindow().GetSwapchain();
		
		//VkCommandBuffer drawCmdBuffer;
		//VkCommandBuffer secondaryCmdBuffer;

		const uint32_t width = Application::Get().GetWindow().GetWidth();
		const uint32_t height = Application::Get().GetWindow().GetHeight();

		//// Begin Command Buffer
		//{
		//	uint32_t frameIndex = swapchain.GetCurrentFrame();

		//	VkCommandBufferBeginInfo beginInfo{};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	beginInfo.pNext = nullptr;
		//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		//	drawCmdBuffer = swapchain.GetCommandBuffer(frameIndex);
		//	secondaryCmdBuffer = s_imguiCommandBuffers.at(frameIndex);

		//	VT_VK_CHECK(vkBeginCommandBuffer(drawCmdBuffer, &beginInfo));
		//}

		//// Begin Render Pass
		//{
		//	VkClearValue clearValues[2];
		//	clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.f } };
		//	clearValues[1].depthStencil = { 1.f, 0 };

		//	VkRenderPassBeginInfo beginInfo{};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		//	beginInfo.pNext = nullptr;
		//	beginInfo.renderPass = swapchain.GetRenderPass();
		//	beginInfo.renderArea.offset.x = 0;
		//	beginInfo.renderArea.offset.y = 0;
		//	beginInfo.renderArea.extent.width = width;
		//	beginInfo.renderArea.extent.height = height;
		//	beginInfo.clearValueCount = 2;
		//	beginInfo.pClearValues = clearValues;
		//	beginInfo.framebuffer = swapchain.GetCurrentFramebuffer();

		//	vkCmdBeginRenderPass(drawCmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		//}

		//// Begin Secondary Command Buffer
		//{
		//	VkCommandBufferInheritanceInfo inheritInfo{};
		//	inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//	inheritInfo.renderPass = swapchain.GetRenderPass();
		//	inheritInfo.framebuffer = swapchain.GetCurrentFramebuffer();

		//	VkCommandBufferBeginInfo beginInfo{};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		//	beginInfo.pInheritanceInfo = &inheritInfo;

		//	VT_VK_CHECK(vkBeginCommandBuffer(secondaryCmdBuffer, &beginInfo));
		//}

		// Viewport
		{
			VkViewport viewport{};
			viewport.x = 0.f;
			viewport.y = (float)height;
			viewport.width = (float)width;
			viewport.height = -(float)height;
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.f;

			//vkCmdSetViewport(secondaryCmdBuffer, 0, 1, &viewport);
		}

		// Scissor
		{
			VkRect2D scissor{};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			//vkCmdSetScissor(secondaryCmdBuffer, 0, 1, &scissor);
		}

		//ImDrawData* drawData = ImGui::GetDrawData();
		//ImGui_ImplVulkan_RenderDrawData(drawData, secondaryCmdBuffer);

		//VT_VK_CHECK(vkEndCommandBuffer(secondaryCmdBuffer));

		//vkCmdExecuteCommands(drawCmdBuffer, 1, &secondaryCmdBuffer);
		//vkCmdEndRenderPass(drawCmdBuffer);

		//VT_VK_CHECK(vkEndCommandBuffer(drawCmdBuffer));

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

	}

	Scope<ImGuiImplementationVolt> ImGuiImplementationVolt::Create()
	{
		return CreateScope<ImGuiImplementationVolt>();
	}

	void ImGuiImplementationVolt::InitializeVulkanData()
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
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		//auto device = GraphicsContextVolt::GetDevice();
		//const auto& swapchain = Application::Get().GetWindow().GetSwapchain();

		////const uint32_t framesInFlight = swapchain.GetMaxFramesInFlight();

		//VT_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &s_descriptorPool));

		//ImGui_ImplVulkan_InitInfo initInfo{};
		//initInfo.Instance = GraphicsContextVolt::Get().GetInstance();
		//initInfo.PhysicalDevice = GraphicsContextVolt::GetPhysicalDevice()->GetHandle();
		//initInfo.Device = GraphicsContextVolt::GetDevice()->GetHandle();
		//initInfo.Queue = device->GetGraphicsQueue();

		//initInfo.DescriptorPool = s_descriptorPool;
		//initInfo.MinImageCount = framesInFlight;
		//initInfo.ImageCount = framesInFlight;
		//initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//initInfo.CheckVkResultFn = Utility::CheckImGuiVulkanResults;

		//ImGui_ImplVulkan_Init(&initInfo, swapchain.GetRenderPass());

		// Create font
		{
			//VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
			//ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			//device->FlushCommandBuffer(commandBuffer);
		}

		//vkDeviceWaitIdle(device->GetHandle());
		//ImGui_ImplVulkan_DestroyFontUploadObjects();

		//s_imguiCommandBuffers.resize(framesInFlight);
		//for (auto& cmdBuffer : s_imguiCommandBuffers)
		//{
		//	cmdBuffer = device->CreateSecondaryCommandBuffer();
		//}
	}

	void ImGuiImplementationVolt::ReleaseVulkanData()
	{
		//vkDeviceWaitIdle(GraphicsContextVolt::GetDevice()->GetHandle());
		//vkDestroyDescriptorPool(GraphicsContextVolt::GetDevice()->GetHandle(), s_descriptorPool, nullptr);
		//ImGui_ImplVulkan_Shutdown();
	}
}
