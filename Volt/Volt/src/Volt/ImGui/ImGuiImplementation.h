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

		inline static ImFont* GetHeaderFont() { return myHeaderFont; }

		static Scope<ImGuiImplementation> Create();

	private:
		ImFont* myFont = nullptr;
		GLFWwindow* myWindowPtr = nullptr;

		inline static ImFont* myHeaderFont = nullptr;
	};
}