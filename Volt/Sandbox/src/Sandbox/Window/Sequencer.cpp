#include "sbpch.h"
#include "Sequencer.h"

Sequencer::Sequencer(Ref<Volt::Scene>& aScene)
	:EditorWindow("Sequencer", false), myCurrentScene(aScene)
{
}

void Sequencer::UpdateMainContent()
{
	if (ImGui::Button("Add Keyframe"))
	{
		KeyFrame keyFrame;
		keyFrame.index = myKeyFrames.size();
		keyFrame.xPos = 10;
		myKeyFrames.emplace_back(keyFrame);
	}
}

void Sequencer::UpdateContent()
{
	UpdateTimeLine();
}

void Sequencer::UpdateTimeLine()
{
	static float scroll_x = 0.0f;
	ImGui::Begin("Timeline");
	ImGui::BeginChild("scrolling", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::SetScrollX(scroll_x);
	if (!myKeyFrames.empty())
	{
		for (int i = 0; i < myKeyFrames.size() - 1; i++)
		{
			std::string name = std::format("X {0}", myKeyFrames[i].index);

			ImGui::Button(name.c_str());
			ImGui::SameLine();
		}
	}
	scroll_x = ImGui::GetScrollX();
	ImGui::EndChild();
	ImGui::End();
}
