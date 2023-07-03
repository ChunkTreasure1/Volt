#include "sbpch.h"
#include "ShaderEditorPanel.h"

#include "Sandbox/Utility/EditorResources.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Rendering/Shader/Shader.h>

ShaderEditorPanel::ShaderEditorPanel()
	: EditorWindow("Shader Editor", true)
{
	myWindowFlags = ImGuiWindowFlags_NoDocking;
}

void ShaderEditorPanel::UpdateMainContent()
{

}

void ShaderEditorPanel::UpdateContent()
{
	UpdateMainPanel();
	UpdateToolbar();
}

void ShaderEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	myCurrentShader = std::reinterpret_pointer_cast<Volt::Shader>(asset);
}

void ShaderEditorPanel::OnClose()
{
	myCurrentShader = nullptr;
}

void ShaderEditorPanel::UpdateToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("##toolbarCharEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ForceWindowDocked(ImGui::GetCurrentWindow());

	if (UI::ImageButton("##Save", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
	{
		if (myCurrentShader)
		{
			Volt::AssetManager::Get().SaveAsset(myCurrentShader);
			UI::Notify(NotificationType::Success, "Saved Shader Definition!", std::format("Saved shader definition {0} to file!", myCurrentShader->GetName()));
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##Load", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Open)), { myButtonSize, myButtonSize }))
	{
		const std::filesystem::path shaderPath = FileSystem::OpenFileDialogue({ { "Shader Definition (*.vtsdef)", "vtsdef" }});
		if (!shaderPath.empty() && FileSystem::Exists(shaderPath))
		{
			myCurrentShader = Volt::AssetManager::GetAsset<Volt::Shader>(shaderPath);
			if (!myCurrentShader || !myCurrentShader->IsValid())
			{
				UI::Notify(NotificationType::Error, "Unable to open shader!", std::format("Unable to open shader definition {0}!", myCurrentShader->assetName));
			}
		}
	}

	ImGui::PopStyleVar(2);
	ImGui::End();
}

void ShaderEditorPanel::UpdateMainPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Main");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	UI::Header("Shader Inputs");

	if (!myCurrentShader)
	{
		return;
	}

	auto& textureDefs = const_cast<std::vector<Volt::ShaderTexture>&>(myCurrentShader->GetResources().shaderTextureDefinitions);

	if (ImGui::Button("Add Texture"))
	{
		textureDefs.emplace_back("newTextureInput", "New Texture");
	}

	const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
	if (ImGui::BeginTable("textures", 2, tableFlags))
	{
		ImGui::TableSetupColumn("Shader Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Editor Name", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableHeadersRow();

		UI::PushId();

		for (auto& [shaderName, editorName] : textureDefs)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			UI::InputText("", shaderName);
			ImGui::TableNextColumn();
			UI::InputText("", editorName);
		}

		UI::PopId();

		ImGui::EndTable();
	}

	ImGui::End();
}
