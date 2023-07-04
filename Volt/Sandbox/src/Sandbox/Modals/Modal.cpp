#include "sbpch.h"
#include "Modal.h"

#include <Volt/Utility/UIUtility.h>

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

	const bool modalOpen = UI::BeginModal(m_strId, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize);

	if (modalOpen)
	{
		DrawModalContent();
		UI::EndModal();
	}

	return modalOpen;
}
