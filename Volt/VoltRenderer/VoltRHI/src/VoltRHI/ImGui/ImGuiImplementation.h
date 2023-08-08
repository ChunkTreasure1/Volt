#pragma once

#include "VoltRHI/Core/RHICommon.h"

struct GLFWwindow;

typedef void* ImTextureID;

namespace Volt::RHI
{
	class Swapchain;
	class Texture2D;
	class Image2D;

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

		virtual ImTextureID GetTextureID(Ref<Texture2D> texture) const = 0;
		virtual ImTextureID GetTextureID(Ref<Image2D> image) const = 0;

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
