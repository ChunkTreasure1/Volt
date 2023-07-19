#pragma once

#include "Volt/Core/Base.h"

#include <imgui.h>

struct GLFWwindow;

namespace Volt
{
	class ImGuiImplementationVolt
	{
	public:
		ImGuiImplementationVolt();
		~ImGuiImplementationVolt();

		void Begin();
		void End();

		static Scope<ImGuiImplementationVolt> Create();

	private:
		void InitializeVulkanData();
		void ReleaseVulkanData();

		ImFont* myFont = nullptr;
		GLFWwindow* myWindowPtr = nullptr;
	};
}
