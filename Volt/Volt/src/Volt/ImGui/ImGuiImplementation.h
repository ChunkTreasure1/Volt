#pragma once

#include "Volt/Core/Base.h"

#include <imgui.h>

struct GLFWwindow;

namespace Volt
{
	class ImGuiImplementation
	{
	public:
		ImGuiImplementation();
		~ImGuiImplementation();

		void Begin();
		void End();

		static Scope<ImGuiImplementation> Create();

	private:
		void InitializeVulkanData();
		void ReleaseVulkanData();

		ImFont* myFont = nullptr;
		GLFWwindow* myWindowPtr = nullptr;
	};
}