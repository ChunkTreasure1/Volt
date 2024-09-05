#include "sbpch.h"
#include "Window/SkeletonEditorPanel.h"

#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Asset/Animation/Skeleton.h>

SkeletonEditorPanel::SkeletonEditorPanel()
	: EditorWindow("Skeleton Editor", false)
{
}

void SkeletonEditorPanel::UpdateMainContent()
{
	UI::Header("Joint Attachments");
	ImGui::Separator();

	if (!m_skeleton)
	{
		return;
	}

	if (ImGui::Button("Save"))
	{
		Volt::AssetManager::SaveAsset(m_skeleton);
	}

	if (ImGui::Button("Add"))
	{
		UI::OpenPopup("addJointAttachmentSkeleton");
		m_jointSearchQuery = "";
		m_activateJointSearch = true;
	}

	auto& jointAttachments = const_cast<Vector<Volt::Skeleton::JointAttachment>&>(m_skeleton->GetJointAttachments());
	const auto totalWidth = ImGui::GetContentRegionAvail().x;

	if (ImGui::BeginTable("AttachmentTable", 3, ImGuiTableFlags_BordersInnerH, ImGui::GetContentRegionAvail()))
	{
		ImGui::TableSetupColumn("Joint");
		ImGui::TableSetupColumn("Attachment Name");

		ImGui::TableHeadersRow();

		int32_t indexToRemove = -1;
		for (int32_t index = 0; auto& attachment : jointAttachments)
		{
			ImGui::TableNextColumn();
		
			auto jointName = m_skeleton->GetNameFromJointIndex(attachment.jointIndex);

			ImGui::PushItemWidth(totalWidth - 11.f);
			const std::string jntId = "##" + std::to_string(UI::GetID());

			ImGui::InputTextString(jntId.c_str(), &jointName, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			ImGui::PushItemWidth(totalWidth - 11.f);

			const std::string attId = "##" + std::to_string(UI::GetID());
			ImGui::InputTextString(attId.c_str(), &attachment.name);

			std::string popupName = "offsetRightclick" + std::to_string(index);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup(popupName.c_str());
			}

			std::string rightClickId = "offsetRightclick" + std::to_string(index);
			if (ImGui::BeginPopupContextItem(rightClickId.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				ImGui::SetWindowSize({ 100.f, 100.f });

				UI::BeginProperties("OFFSET");
				ImGui::Text("OFFSET");

				UI::Property("Pos", attachment.positionOffset);
				UI::Property("Rot", attachment.rotationOffset);

				UI::EndProperties();
				ImGui::EndPopup();
			}

			ImGui::PopItemWidth();

			ImGui::TableNextColumn();
			if (ImGui::Button((std::string("-##") + std::to_string(index)).c_str(), { 22.f, 22.f }))
			{
				indexToRemove = index;
			}

			index++;
		}

		if (indexToRemove > -1)
		{
			jointAttachments.erase(jointAttachments.begin() + indexToRemove);
		}

		ImGui::EndTable();
	}

	AddJointAttachmentPopup();
}

void SkeletonEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (m_skeleton)
	{
		Volt::AssetManager::SaveAsset(std::reinterpret_pointer_cast<Volt::Asset>(m_skeleton));
	}

	m_skeleton = std::reinterpret_pointer_cast<Volt::Skeleton>(asset);
}

void SkeletonEditorPanel::OnOpen()
{
}

void SkeletonEditorPanel::OnClose()
{
	m_skeleton = nullptr;
}

void SkeletonEditorPanel::AddJointAttachmentPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });

	if (UI::BeginPopup("addJointAttachmentSkeleton", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		Vector<std::string> jointNames;
		for (const auto& joint : m_skeleton->GetJoints())
		{
			jointNames.emplace_back(joint.name);
		}

		// Search bar
		{
			bool t;
			EditorUtils::SearchBar(m_jointSearchQuery, t, m_activateJointSearch);
			if (m_activateJointSearch)
			{
				m_activateJointSearch = false;
			}
		}

		if (!m_jointSearchQuery.empty())
		{
			jointNames = UI::GetEntriesMatchingQuery(m_jointSearchQuery, jointNames);
		}

		// List child
		{
			auto& jointAttachments = const_cast<Vector<Volt::Skeleton::JointAttachment>&>(m_skeleton->GetJointAttachments());

			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			for (const auto& name : jointNames)
			{
				const std::string id = name + "##" + std::to_string(UI::GetID());

				UI::ShiftCursor(4.f, 0.f);
				UI::RenderMatchingTextBackground(m_jointSearchQuery, name, EditorTheme::MatchingTextBackground);
				if (ImGui::MenuItem(id.c_str()))
				{
					auto& newAttachment = jointAttachments.emplace_back();
					newAttachment.name = "New Attachment";
					newAttachment.jointIndex = m_skeleton->GetJointIndexFromName(name);

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndChild();
		}

		UI::EndPopup();
	}
}
