#pragma once

#include "RHIModule/Core/RHICommon.h"
#include "RHIModule/Core/RHIInterface.h"

struct GLFWwindow;

typedef void* ImTextureID;
struct ImFont;
struct ImGuiContext;

namespace Volt::RHI
{
	class Swapchain;
	class Image2D;

	struct ImGuiCreateInfo
	{
		GLFWwindow* window = nullptr;
		WeakPtr<Swapchain> swapchain;
	};

	class VTRHI_API ImGuiImplementation : public RHIInterface
	{
	public:
		virtual ~ImGuiImplementation();

		VT_DELETE_COMMON_OPERATORS(ImGuiImplementation);

		void Begin();
		void End();

		void SetDefaultFont(ImFont* font);
		ImGuiContext* GetContext() const;

		virtual ImTextureID GetTextureID(RefPtr<Image2D> image) const = 0;
		virtual ImFont* AddFont(const std::filesystem::path& fontPath, float pixelSize) = 0;

		static RefPtr<ImGuiImplementation> Create(const ImGuiCreateInfo& createInfo);
		static ImGuiImplementation& Get();

	protected:
		ImGuiImplementation();

		void Initialize();

		virtual void BeginAPI() = 0;
		virtual void EndAPI() = 0;

		virtual void InitializeAPI(ImGuiContext* context) {}
		virtual void ShutdownAPI() {}

	private:
		inline static ImGuiImplementation* s_instance = nullptr;
		ImFont* m_defaultFont = nullptr;
	};
}
