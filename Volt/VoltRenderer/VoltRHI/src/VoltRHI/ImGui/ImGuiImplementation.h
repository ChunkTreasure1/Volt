#pragma once

#include "VoltRHI/Core/RHICommon.h"

struct GLFWwindow;

namespace Volt::RHI
{
	class Swapchain;

	struct ImGuiCreateInfo
	{
		GLFWwindow* window = nullptr;
		Weak<Swapchain> swapchain;
	};

	class ImGuiImplementation
	{
	public:
		virtual ~ImGuiImplementation();

		VT_DELETE_COMMON_OPERATORS(ImGuiImplementation);

		void Begin();
		void End();

		static Ref<ImGuiImplementation> Create(const ImGuiCreateInfo& createInfo);

	protected:
		ImGuiImplementation();

		virtual void BeginAPI() = 0;
		virtual void EndAPI() = 0;

		virtual void InitializeAPI() {}
		virtual void ShutdownAPI() {}

	private:
		void Initialize();
	};
}