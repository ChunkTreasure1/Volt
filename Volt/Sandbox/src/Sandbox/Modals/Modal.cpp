#include "sbpch.h"
#include "Modal.h"

#include <Volt/Utility/UIUtility.h>

#include "Sandbox/Utility/Theme.h"

Modal::Modal(const std::string& strId)
	: m_strId(strId)
{

}

void Modal::Open()
{
	UI::OpenModal(m_strId);
	m_wasOpenLastFrame = false;

	OnOpen();
}

void Modal::Close()
{
	ImGui::CloseCurrentPopup();

	OnClose();
}

bool Modal::Update()
{
	if (!m_wasOpenLastFrame)
	{
		const auto viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetCenter());

		m_wasOpenLastFrame = true;
	}

	// Default modal style
	UI::ScopedColor backgroundColorChild{ ImGuiCol_PopupBg, EditorTheme::ToNormalizedRGB(26.f, 26.f, 26.f) };
	UI::ScopedColor frameColor{ ImGuiCol_Header, EditorTheme::ToNormalizedRGB(47.f, 47.f, 47.f) };
	UI::ScopedColor frameColorActive{ ImGuiCol_FrameBgActive, EditorTheme::ToNormalizedRGB(47.f, 47.f, 47.f) };
	UI::ScopedColor frameColorHovered{ ImGuiCol_FrameBgHovered, EditorTheme::ToNormalizedRGB(47.f, 47.f, 47.f) };

	// Default button style
	UI::ScopedButtonColor defaultButtonColor{ EditorTheme::Buttons::DefaultButton };

	const bool modalOpen = UI::BeginModal(m_strId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize);

	if (modalOpen)
	{
		DrawModalContent();
		UI::EndModal();
	}

	return modalOpen;
}
