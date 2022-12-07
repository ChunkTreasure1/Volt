#include "vtpch.h"
#include "UIUtility.h"

#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/ImGui/ImGuiImplementation.h"

#include <backends/imgui_impl_dx11.h>

ImTextureID UI::GetTextureID(Ref<Volt::Texture2D> texture)
{
	return (ImTextureID)texture->GetImage()->GetSRV().Get();
}

ImTextureID UI::GetTextureID(Ref<Volt::Image2D> texture)
{
	return (ImTextureID)texture->GetSRV().Get();
}

ImTextureID UI::GetTextureID(Volt::Texture2D* texture)
{
	return (ImTextureID)texture->GetImage()->GetSRV().Get();
}

void UI::Header(const std::string& text)
{
	ImGui::PushFont(Volt::ImGuiImplementation::GetHeaderFont());
	ImGui::TextUnformatted(text.c_str());
	ImGui::PopFont();
}
