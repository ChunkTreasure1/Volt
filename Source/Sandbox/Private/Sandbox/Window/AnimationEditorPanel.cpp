#include "sbpch.h"
#include "Window/AnimationEditorPanel.h"

#include <AssetSystem/AssetManager.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Utility/UIUtility.h>

AnimationEditorPanel::AnimationEditorPanel()
	: EditorWindow("Animation Editor", false)
{
}

void AnimationEditorPanel::UpdateMainContent()
{
	if (!m_animation)
	{
		return;
	}

	const auto duration = m_animation->GetDuration();
	const int32_t stepCount = (int32_t)m_animation->GetFrameCount();

	if (ImGui::Button("Save"))
	{
		Volt::AssetManager::SaveAsset(m_animation);
	}

	if (ImGui::BeginTable("timelineTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextColumn();
		if (ImGui::BeginChild("eventsChild", { ImGui::GetColumnWidth(), ImGui::GetContentRegionAvail().y }, true))
		{
			UI::Header("Created Events");
			ImGui::Separator();
			if (m_animation->HasEvents())
			{
				const auto& events = m_animation->GetEvents();
				for (int index = 0; index < events.size(); index++)
				{
					const auto id = UI::GetID();
					bool selected = false;

					ImGui::Selectable(std::format("{0}: ", events[index].name).c_str(), &selected, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(150, 25));
					ImGui::SameLine();
					ImGui::DragInt(std::format("-##rem{0}", id).c_str(), (int*)&events[index].frame);

					std::string popupName = "eventPopup" + std::to_string(index);
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup(popupName.c_str());
					}

					std::string rightClickId = "eventRightClick" + std::to_string(index);
					if (ImGui::BeginPopupContextItem(rightClickId.c_str(), ImGuiPopupFlags_MouseButtonRight))
					{
						if (ImGui::MenuItem("Remove"))
						{
							m_animation->RemoveEvent(events[index].name, events[index].frame);
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							break;
						}
						ImGui::EndPopup();
					}

				}
			}

			ImGui::EndChild();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("keyframeChild", ImGui::GetContentRegionAvail()))
		{
			if (ImGui::BeginChild("timeChild", { ImGui::GetContentRegionAvail().x, 35.f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::TextUnformatted("0.00");
				ImGui::SameLine();

				auto drawList = ImGui::GetWindowDrawList();
				ImVec2 windowPos = ImGui::GetWindowPos();
				ImVec2 windowSize = ImGui::GetWindowSize();
				ImGuiIO& io = ImGui::GetIO();

				drawList->AddRectFilled(windowPos, windowPos + windowSize, IM_COL32(50, 50, 50, 255));

				UI::ScopedColor color{ ImGuiCol_Button, { 1.f, 1.f, 1.f, 1.f } };
				{
					constexpr float padding = 3.f;
					UI::ScopedStyleFloat2 itemPadding{ ImGuiStyleVar_ItemSpacing, { 3.f, 0.f } };

					const float stepSize = windowSize.x / stepCount;
					for (int32_t i = 0; i < stepCount; i++)
					{
						//ImGui::Button(("K##" + std::to_string(i)).c_str(), { stepSize, 20.f });
						ImVec2 keyMinPos = ImVec2(windowPos.x + (stepSize * i) + padding, windowPos.y);
						ImVec2 keyMaxPos = ImVec2(windowPos.x + (stepSize * (i + 1)) - padding, windowPos.y + windowSize.y);

						if (m_selectedKeyFrame == i)
						{
							drawList->AddRectFilled(keyMinPos, keyMaxPos, IM_COL32(100, 100, 100, 255));
						}
						else
						{
							drawList->AddRectFilled(keyMinPos, keyMaxPos, IM_COL32(255, 255, 255, 255));
						}

						ImGui::ItemAdd(ImRect(keyMinPos, keyMaxPos), i);

						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						{
							m_selectedKeyFrame = i;
						}

						if (ImGui::IsItemHovered())
						{
							drawList->AddText(io.MousePos + ImVec2(0, -20), IM_COL32(255, 100, 100, 255), std::to_string(i).c_str());
						}

						if (ImGui::BeginPopupContextItem(("item##" + std::to_string(i)).c_str(), ImGuiPopupFlags_MouseButtonRight))
						{
							if (ImGui::MenuItem("Add Event"))
							{
								UI::OpenModal("Create Animation Event");
								m_addAnimEventData = {};
								m_addAnimEventData.frame = i;
							}

							ImGui::EndPopup();
						}

						ImGui::SameLine();
					}
				}

				ImGui::Text("%.2f", duration);

				ImGui::EndChild();
			}

			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	AddAnimationEventModal();
}

void AnimationEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (m_animation)
	{
		Volt::AssetManager::SaveAsset(m_animation);
	}

	m_animation = std::reinterpret_pointer_cast<Volt::Animation>(asset);
}

void AnimationEditorPanel::OnOpen()
{
}

void AnimationEditorPanel::OnClose()
{
	m_animation = nullptr;
}

void AnimationEditorPanel::AddAnimationEventModal()
{
	if (UI::BeginModal("Create Animation Event", ImGuiWindowFlags_AlwaysAutoResize))
	{
		UI::PushID();
		if (UI::BeginProperties("animEvent"))
		{
			UI::Property("Name", m_addAnimEventData.name);
			UI::EndProperties();
		}
		UI::PopID();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			m_animation->AddEvent(m_addAnimEventData.name, m_addAnimEventData.frame);
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}
