#include "sbpch.h"
#include "Timeline.h"

#include "Volt/Vision/VisionComponents.h"
#include "Volt/Asset/Importers/AssetImporter.h"
#include "Volt/Utility/UIUtility.h"
#include "Volt/Input/Input.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Rendering/Camera/Camera.h"

Timeline::Timeline(Ref<Volt::Scene>& aScene, EditorCameraController* editorCamera)
	:EditorWindow("Timeline", true), myCurrentScene(aScene), myEditorCamera(editorCamera)
{
	m_windowFlags = ImGuiWindowFlags_MenuBar;
}

void Timeline::UpdateMainContent()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				SaveAsset();
			}

			if (ImGui::MenuItem("Load"))
			{
				//LoadAsset();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Track"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Animation Track"))
				{
					myTimelinePreset->myTracks.emplace_back(Volt::Track(Volt::TrackType::T_Animation));
				}

				if (ImGui::MenuItem("Clip Track"))
				{
					myTimelinePreset->myTracks.emplace_back(Volt::Track(Volt::TrackType::T_Clip));
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void Timeline::UpdateContent()
{
	UpdateTimelineTracks();
	UpdateTimeLine();

	if (myPlayingSequence)
	{
		PlaySequence();
	}

	if (myPreviewOn)
	{
		PreviewTimeline();
	}


}

void Timeline::OpenAsset(Ref<Volt::Asset> asset)
{
	myTimelinePreset = Volt::AssetManager::GetAsset<Volt::TimelinePreset>(asset->handle);
}

void Timeline::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(Timeline::OnUpdateEvent));

	if (Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL))
	{
		dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(Timeline::CameraQuickshotKeyframe));
	}
}

bool Timeline::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	myDeltaTime = e.GetTimestep();
	return false;
}

int Timeline::GetFrameFromPos(ImVec2 pos)
{
	const float posPercentage = (pos.x - myTimelinePos.x) / myTimelineSize.x;
	const int resultFrame = static_cast<int32_t>(myTimelinePreset->maxLength * posPercentage);

	return resultFrame;
}

void Timeline::SaveAsset()
{
	Volt::AssetManager::Get().SaveAsset(myTimelinePreset);
}

void Timeline::PlaySequence()
{
	myCurrentScene->GetTimelinePlayer().Update(myDeltaTime, myCurrentScene.get());

	const float pixelPerSecond = myTimelineSize.x / (myTimelinePreset->maxLength / 30);
	const float markerPixelStep = (pixelPerSecond * myDeltaTime);

	myMPoint3.x += markerPixelStep;
	if (myMPoint3.x > myTimelinePos.x + myTimelineSize.x)
	{
		myMPoint3.x = myTimelinePos.x;
	}

	myMPoint1 = ImVec2(myMPoint3.x - myMarkerWidth, myTimelinePos.y + 5);
	myMPoint2 = ImVec2(myMPoint3.x + myMarkerWidth, myTimelinePos.y + 5);
}

void Timeline::OnPlay()
{
	myMPoint3.x = myTimelinePos.x;
	myPlayingSequence = true;
	myCurrentScene->GetTimelinePlayer().StartTimeline(*myTimelinePreset, myCurrentScene.get());
}

void Timeline::OnStop()
{
	myPlayingSequence = false;
	myCurrentScene->GetTimelinePlayer().OnRuntimeEnd(myCurrentScene.get());
	myCurrentScene->GetTimelinePlayer().StopTimeline();
}

void Timeline::HandleTimelineInfo()
{
	const ImVec2 timelineWindowSize = ImGui::GetContentRegionAvail();

	if (ImGui::IsMouseHoveringRect(myTimelinePos, myTimelinePos + timelineWindowSize))
	{
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
		{
			if (mySelectedTrack)
			{
				if (mySelectedTrack->trackType == Volt::TrackType::T_Animation)
				{
					if (ImGui::MenuItem("Add Keyframe"))
					{
						AddKeyframe();
					}
				}
				else
				{
					if (ImGui::MenuItem("Add Clip"))
					{
						AddClip(0);
					}
				}
			}
			ImGui::EndPopup();
		}
	}

	ImGui::ItemAdd(ImRect(myTimelinePos, myTimelinePos + timelineWindowSize), ImGuiID(10000));

	if (auto ptr = UI::DragDropTarget("scene_entity_hierarchy"))
	{
		Volt::EntityID entityId = *(Volt::EntityID*)ptr;
		AddClip(entityId);
	}
}

void Timeline::HandleTrackContent()
{
	auto style = ImGui::GetStyle();

	for (int32_t i = 0; i < static_cast<int32_t>(myTimelinePreset->myTracks.size()); i++)
	{
		if (myTimelinePreset->myTracks[i].trackType == Volt::TrackType::T_Animation)
		{
			DrawKeyframesOnTrack(i);
		}
		else if (myTimelinePreset->myTracks[i].trackType == Volt::TrackType::T_Clip)
		{
			DrawClipsOnTrack(i);
		}
	}
}

void Timeline::HandleSelectedKeyframe()
{
	if (mySelectedKeyframe != nullptr && mySelectedClip == nullptr && myIsDraging && !myMovingMarker)
	{
		mySelectedKeyframe->timelineXPos = ImGui::GetMousePos().x;
		mySelectedKeyframe->timelineXPos = glm::clamp(mySelectedKeyframe->timelineXPos, myTimelinePos.x, myTimelinePos.x + myTimelineSize.x);

		const float frameTime = 1.f / 30.f;
		mySelectedKeyframe->time = mySelectedKeyframe->frame * frameTime;
		mySelectedKeyframe->frame = GetFrameFromPos(ImVec2(mySelectedKeyframe->timelineXPos, 0));

		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	}
}

void Timeline::HandleSelectedClip()
{
	if (mySelectedClip != nullptr && myIsDraging && !myMovingMarker && !myIsStretching)
	{
		float timeStep = (ImGui::GetIO().MouseDelta.x / myTimelineSize.x) * myTimelinePreset->maxLength / 30;

		mySelectedClip->startTime += timeStep;
		mySelectedClip->endTime += timeStep;

		if (mySelectedClip->startTime < 0)
		{
			mySelectedClip->startTime = 0;
			mySelectedClip->endTime -= timeStep;
		}
	}
}

void Timeline::SortTrack(Volt::Track& track)
{
	if (track.trackType == Volt::TrackType::T_Animation)
	{
		std::sort(std::execution::par, track.keyframes.begin(), track.keyframes.end(),
			[](const Volt::Keyframe& lhsKeyFrame, const Volt::Keyframe& rhsKeyFrame)
		{
			return lhsKeyFrame.time < rhsKeyFrame.time;
		});
	}
	else if (track.trackType == Volt::TrackType::T_Clip)
	{
		std::sort(std::execution::par, track.clips.begin(), track.clips.end(),
			[](const Volt::Clip& lhsClip, const Volt::Clip& rhsClip)
		{
			return lhsClip.startTime < rhsClip.startTime;
		});
	}
}

bool Timeline::CameraQuickshotKeyframe(Volt::KeyPressedEvent& e)
{
	if (e.GetKeyCode() != VT_KEY_Q) { return false; }

	if (mySelectedTrack == nullptr)
	{
		UI::Notify(NotificationType::Error, "Timeline", "No track selected to record!");
		return false;
	}

	if (mySelectedTrack->targetEntity == Volt::Entity::NullID())
	{
		UI::Notify(NotificationType::Error, "Timeline", "No entity in selected track!");
		return false;
	}

	//AddFrame
	Volt::Keyframe newKeyframe;
	newKeyframe.frame = GetFrameFromPos(myMPoint3);

	newKeyframe.position = myEditorCamera->GetCamera()->GetPosition();
	newKeyframe.rotation = myEditorCamera->GetCamera()->GetRotation();

	const float frameTime = 1.f / 30.f;
	newKeyframe.time = newKeyframe.frame * frameTime;

	newKeyframe.timelineXPos = myTimelinePos.x + (myTimelineSize.x / myTimelinePreset->maxLength) * newKeyframe.frame;

	mySelectedTrack->keyframes.emplace_back(newKeyframe);

	SortTrack(*mySelectedTrack);
	return true;
}

void Timeline::AddKeyframe()
{
	if (mySelectedTrack == nullptr)
	{
		UI::Notify(NotificationType::Error, "Timeline", "No track selected to record!");
		return;
	}

	if (mySelectedTrack->targetEntity == Volt::Entity::NullID())
	{
		UI::Notify(NotificationType::Error, "Timeline", "No entity in selected track!");
		return;
	}

	//AddFrame
	Volt::Keyframe newKeyframe;
	newKeyframe.frame = GetFrameFromPos(myMPoint3);

	Volt::Entity targetEnt = myCurrentScene->GetEntityFromUUID(mySelectedTrack->targetEntity);
	newKeyframe.position = targetEnt.GetPosition();
	newKeyframe.rotation = targetEnt.GetRotation();

	const float frameTime = 1.f / 30.f;
	newKeyframe.time = newKeyframe.frame * frameTime;

	newKeyframe.timelineXPos = myTimelinePos.x + (myTimelineSize.x / myTimelinePreset->maxLength) * newKeyframe.frame;

	mySelectedTrack->keyframes.emplace_back(newKeyframe);

	SortTrack(*mySelectedTrack);
}

void Timeline::AddClip(Volt::EntityID entityId)
{
	if (mySelectedTrack == nullptr)
	{
		UI::Notify(NotificationType::Error, "Timeline", "No track selected to record!");
		return;
	}

	Volt::Entity cameraEnt = myCurrentScene->GetEntityFromUUID(entityId);
	if (!cameraEnt.HasComponent<Volt::VisionCameraComponent>())
	{
		UI::Notify(NotificationType::Error, "Timeline", "Entity needs to be a Vision Camera");
		return;
	}

	Volt::Clip newClip;
	newClip.activeCamera = cameraEnt.GetID();

	const float markerTime = ((myMPoint3.x - myTimelinePos.x) / myTimelineSize.x) * (myTimelinePreset->maxLength / 30);
	newClip.startTime = markerTime - 1.f;
	newClip.endTime = markerTime + 1.f;

	mySelectedTrack->clips.push_back(newClip);
}

void Timeline::PreviewTimeline()
{
	const float markerTimePercentage = (myMPoint3.x - myTimelinePos.x) / myTimelineSize.x;
	const float timeStamp = (myTimelinePreset->maxLength / 30) * markerTimePercentage;

	myCurrentScene->GetTimelinePlayer().GetPreviewOnTime(*myTimelinePreset, timeStamp, *mySelectedTrack, myCurrentScene.get());
}

void Timeline::DrawEntityTracks(ImDrawList& drawlist, int trackIndex)
{
	const float recordButtonWidth = 23.f;

	if (myTimelinePreset->myTracks[trackIndex].trackType == Volt::TrackType::T_Animation)
	{
		UI::PropertyEntity(myCurrentScene, myTimelinePreset->myTracks[trackIndex].targetEntity, myTrackWindowSize.x - recordButtonWidth - ImGui::GetStyle().ItemSpacing.x, nullptr);
	}
	else
	{
		std::string id = "##" + std::to_string(trackIndex);
		std::string clipTrackName = "ClipTrack";

		ImGui::PushItemWidth(myTrackWindowSize.x - recordButtonWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::InputTextString(id.c_str(), &clipTrackName, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();
	}

	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::MenuItem("Remove Track"))
		{
			myTimelinePreset->myTracks.erase(myTimelinePreset->myTracks.begin() + trackIndex);
		}
		ImGui::EndPopup();
	}

	myIndividualTrackSize = ImGui::GetItemRectSize();

	if (mySelectedTrack == &myTimelinePreset->myTracks[trackIndex])
	{
		mySelectedTrackMinPos = ImGui::GetItemRectMin();
		mySelectedTrackMaxPos = ImGui::GetItemRectMax();

		drawlist.AddRectFilled(mySelectedTrackMinPos, mySelectedTrackMaxPos, IM_COL32(255, 0, 0, 100));
	}

	ImGui::SameLine();

	std::string recordButtonId = "O##RecordButton" + std::to_string(trackIndex);

	//Record Button
	if (ImGui::Button(recordButtonId.c_str(), ImVec2(recordButtonWidth, recordButtonWidth)))
	{
		if (mySelectedTrack == &myTimelinePreset->myTracks[trackIndex])
		{
			mySelectedTrack = nullptr;
		}
		else
		{
			mySelectedTrack = &myTimelinePreset->myTracks[trackIndex];
		}
	}
}

void Timeline::DrawKeyframesOnTrack(int trackIndex)
{
	auto drawlist = ImGui::GetWindowDrawList();
	auto style = ImGui::GetStyle();

	if (!myTimelinePreset->myTracks.empty())
	{
		if (myTimelinePreset->myTracks[trackIndex].keyframes.empty()) { return; }

		HandleSelectedKeyframe();

		for (size_t keyIndex = 0; keyIndex < myTimelinePreset->myTracks[trackIndex].keyframes.size(); keyIndex++)
		{
			auto keyframe = myTimelinePreset->myTracks[trackIndex].keyframes[keyIndex];

			const float startDrawHeight = myTimelinePos.y + myTimelineSize.y;
			const ImVec2 trackMinPos = ImVec2(myTimelinePos.x, startDrawHeight + (style.FramePadding.y + ((myIndividualTrackSize.y + style.ItemSpacing.y) * trackIndex)));

			//Draw Frame
			const ImVec2 down = ImVec2(keyframe.timelineXPos, trackMinPos.y + myIndividualTrackSize.y - 3);
			const ImVec2 up = ImVec2(keyframe.timelineXPos, trackMinPos.y + 3);
			const ImVec2 left = ImVec2(keyframe.timelineXPos - 5, trackMinPos.y + myIndividualTrackSize.y * 0.5f);
			const ImVec2 right = ImVec2(keyframe.timelineXPos + 5, trackMinPos.y + myIndividualTrackSize.y * 0.5f);

			if (mySelectedKeyframe == &myTimelinePreset->myTracks[trackIndex].keyframes[keyIndex])
			{
				drawlist->AddQuadFilled(down, left, up, right, IM_COL32(255, 150, 150, 255));
			}
			else
			{
				drawlist->AddQuadFilled(down, left, up, right, IM_COL32(255, 255, 255, 255));
			}

			ImGui::ItemAdd(ImRect(ImVec2(up.x - 5, up.y), ImVec2(down.x + 5, down.y)), ImGuiID(keyIndex));

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				mySelectedKeyframe = &myTimelinePreset->myTracks[trackIndex].keyframes[keyIndex];
				mySelectedClip = nullptr;
			}

			if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left])
			{
				if (ImGui::IsItemHovered() && !myMovingMarker && !myIsDraging)
				{
					myIsDraging = true;
					mySelectedKeyframe = &myTimelinePreset->myTracks[trackIndex].keyframes[keyIndex];
				}
			}
			else
			{
				myIsDraging = false;
			}

			if (mySelectedKeyframe != nullptr)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_Delete) && mySelectedKeyframe == &myTimelinePreset->myTracks[trackIndex].keyframes[keyIndex])
				{
					mySelectedKeyframe = nullptr;
					myTimelinePreset->myTracks[trackIndex].keyframes.erase(myTimelinePreset->myTracks[trackIndex].keyframes.begin() + keyIndex);
				}
			}
		}
	}
}

void Timeline::DrawClipsOnTrack(int trackIndex)
{
	auto drawlist = ImGui::GetWindowDrawList();
	auto style = ImGui::GetStyle();

	if (!myTimelinePreset->myTracks.empty())
	{
		if (myTimelinePreset->myTracks[trackIndex].clips.empty()) { return; }

		HandleSelectedClip();

		const float startDrawHeight = myTimelinePos.y + myTimelineSize.y;
		const ImVec2 trackMinPos = ImVec2(myTimelinePos.x, startDrawHeight + (style.FramePadding.y + ((myIndividualTrackSize.y + style.ItemSpacing.y) * trackIndex)));

		//Draw Clip Rects
		for (size_t clipIndex = 0; clipIndex < myTimelinePreset->myTracks[trackIndex].clips.size(); clipIndex++)
		{
			auto& currClip = myTimelinePreset->myTracks[trackIndex].clips[clipIndex];

			//Draw
			const float trackXStartPos = myTimelinePos.x + ((currClip.startTime / (myTimelinePreset->maxLength / 30)) * myTimelineSize.x);
			const float trackXEndPos = myTimelinePos.x + ((currClip.endTime / (myTimelinePreset->maxLength / 30)) * myTimelineSize.x);

			if (mySelectedClip == &myTimelinePreset->myTracks[trackIndex].clips[clipIndex])
			{
				drawlist->AddRectFilledMultiColor(ImVec2(trackXStartPos, trackMinPos.y), ImVec2(trackXEndPos, trackMinPos.y + myIndividualTrackSize.y), IM_COL32(150, 100, 100, 255), IM_COL32(150, 100, 100, 255), IM_COL32(100, 100, 100, 255), IM_COL32(100, 100, 100, 255));
				drawlist->AddRect(ImVec2(trackXStartPos, trackMinPos.y), ImVec2(trackXEndPos, trackMinPos.y + myIndividualTrackSize.y), IM_COL32(255, 100, 100, 255));
			}
			else
			{
				drawlist->AddRectFilledMultiColor(ImVec2(trackXStartPos, trackMinPos.y), ImVec2(trackXEndPos, trackMinPos.y + myIndividualTrackSize.y), IM_COL32(150, 150, 150, 255), IM_COL32(150, 150, 150, 255), IM_COL32(100, 100, 100, 255), IM_COL32(100, 100, 100, 255));
			}

			Volt::Entity camEnt = myCurrentScene->GetEntityFromUUID(currClip.activeCamera);
			std::string entName;
			if (camEnt)
			{
				entName = camEnt.GetComponent<Volt::TagComponent>().tag;
			}
			else
			{
				entName = "Null";
			}

			drawlist->AddText(ImVec2(trackXStartPos, trackMinPos.y + 3), IM_COL32(10, 10, 10, 255), entName.c_str());

			//Selectable Area
			ImGui::ItemAdd(ImRect(ImVec2(trackXStartPos, trackMinPos.y), ImVec2(trackXEndPos, trackMinPos.y + myIndividualTrackSize.y)), ImGuiID(clipIndex));

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !myIsStretching)
			{
				mySelectedClip = &myTimelinePreset->myTracks[trackIndex].clips[clipIndex];
				mySelectedKeyframe = nullptr;
			}


			if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left])
			{
				if (ImGui::IsItemHovered() && !myIsDraging && !myIsStretching)
				{
					myIsDraging = true;
					mySelectedClip = &myTimelinePreset->myTracks[trackIndex].clips[clipIndex];
				}
			}
			else
			{
				myIsDraging = false;
				SortTrack(myTimelinePreset->myTracks[trackIndex]);
			}

			if (mySelectedClip != nullptr)
			{
				if (ImGui::IsKeyPressed(ImGuiKey_Delete) && mySelectedClip == &myTimelinePreset->myTracks[trackIndex].clips[clipIndex])
				{
					mySelectedClip = nullptr;
					myTimelinePreset->myTracks[trackIndex].clips.erase(myTimelinePreset->myTracks[trackIndex].clips.begin() + clipIndex);
					SortTrack(myTimelinePreset->myTracks[trackIndex]);
				}
			}
		}

		//Draw Blend Rects
		for (size_t clipIndex = 0; clipIndex < myTimelinePreset->myTracks[trackIndex].clips.size(); clipIndex++)
		{
			if (clipIndex + 1 < myTimelinePreset->myTracks[trackIndex].clips.size())
			{
				const auto& currClip = myTimelinePreset->myTracks[trackIndex].clips[clipIndex];
				const auto& nextClip = myTimelinePreset->myTracks[trackIndex].clips[clipIndex + 1];
				if (nextClip.startTime < currClip.endTime)
				{
					const float blendXStartPos = myTimelinePos.x + ((nextClip.startTime / (myTimelinePreset->maxLength / 30)) * myTimelineSize.x);
					const float blendXEndPos = myTimelinePos.x + ((currClip.endTime / (myTimelinePreset->maxLength / 30)) * myTimelineSize.x);
					const float blendXMidPos = blendXEndPos - ((blendXEndPos - blendXStartPos) * 0.5f);

					const ImVec2 blendMinPos = ImVec2(blendXStartPos, trackMinPos.y);
					const ImVec2 blendMaxPos = ImVec2(blendXEndPos, trackMinPos.y + myIndividualTrackSize.y);

					drawlist->AddRectFilledMultiColor(blendMinPos, blendMaxPos, IM_COL32(100, 100, 100, 90), IM_COL32(100, 100, 100, 90), IM_COL32(50, 50, 50, 90), IM_COL32(50, 50, 50, 90));

					drawlist->AddBezierCurve(blendMinPos, ImVec2(blendXMidPos, blendMaxPos.y), ImVec2(blendXMidPos, blendMinPos.y), blendMaxPos, IM_COL32(255, 255, 255, 90), 1);
				}
			}
		}
	}
}

void Timeline::UpdateTimelineTracks()
{
	ImGui::Begin("Tracks");

	myTrackWindowSize = ImGui::GetContentRegionAvail();
	myTrackWindowPos = ImGui::GetCursorScreenPos();

	if (ImGui::Button(ICON_FA_PLAY, ImVec2(30, 30)))
	{
		OnPlay();
	}

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_STOP, ImVec2(30, 30)))
	{
		OnStop();
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(50);

	if (ImGui::DragInt("##MaxLength", &myTimelinePreset->maxLength, 1, 0))
	{
		for (size_t i = 0; i < myTimelinePreset->myTracks.size(); i++)
		{
			for (size_t j = 0; j < myTimelinePreset->myTracks[i].keyframes.size(); j++)
			{
				myTimelinePreset->myTracks[i].keyframes[j].timelineXPos = myTimelinePos.x + (myTimelineSize.x / myTimelinePreset->maxLength) * myTimelinePreset->myTracks[i].keyframes[j].frame;
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button(ICON_FA_RECORD_VINYL, ImVec2(30, myTimelineSize.y)))
	{
		myPreviewOn = !myPreviewOn;
	}

	ImGui::PopItemWidth();

	//Tracks
	if (myTimelinePreset != nullptr)
	{
		ImGui::BeginChild("TracksC", ImVec2(myTrackWindowPos.x + myTrackWindowSize.x, myTrackWindowSize.y - 30 - ImGui::GetStyle().ItemSpacing.y), false);

		const ImVec2 trackChildPos = ImGui::GetCursorScreenPos();
		myIndividualTrackSize = ImVec2(0, 0);
		auto trackDrawList = ImGui::GetWindowDrawList();

		for (int i = 0; i < myTimelinePreset->myTracks.size(); i++)
		{
			DrawEntityTracks(*trackDrawList, i);
		}

		ImGui::EndChild();
	}

	ImGui::End();
}

void Timeline::UpdateTimeLine()
{
	ImGui::Begin("TimelinePanel");

	myMarkerWidth = 7.f;

	auto drawlist = ImGui::GetWindowDrawList();
	auto canvasSize = ImGui::GetContentRegionAvail();
	auto canvasPos = ImGui::GetCursorScreenPos();

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	const ImVec4& coreBackgroundColor = style.Colors[ImGuiCol_FrameBg];
	const ImVec2 coreBackgroundSize = ImVec2{ canvasSize.x, canvasSize.y };

	auto timelineBackgroundColor = IM_COL32(50, 50, 50, 255);
	auto timelinePreivewOnBackgroundColor = IM_COL32(50, 20, 20, 255);

	myTimelineSize = ImVec2{ canvasSize.x, 30 };
	myTimelinePos = canvasPos;

	if (myPreviewOn)
	{
		drawlist->AddRectFilled(canvasPos, canvasPos + myTimelineSize, ImColor(timelinePreivewOnBackgroundColor));
	}
	else
	{
		drawlist->AddRectFilled(canvasPos, canvasPos + myTimelineSize, ImColor(timelineBackgroundColor));
	}

	const ImVec2 trackSize = ImVec2{ canvasSize.x, 22 };
	const ImVec2 trackCanvasPos = ImVec2{ canvasPos.x, canvasPos.y + myTimelineSize.y };

	//Draws Timeline background
	drawlist->AddRectFilled(trackCanvasPos, trackCanvasPos + coreBackgroundSize, ImColor(coreBackgroundColor));

	if (myTimelinePreset)
	{
		if (mySelectedTrack)
		{
			drawlist->AddRectFilled(ImVec2(trackCanvasPos.x, mySelectedTrackMinPos.y), ImVec2(trackCanvasPos.x + myTimelineSize.x, mySelectedTrackMaxPos.y), IM_COL32(60, 60, 60, 200), 3);
		}
	}

	const float lineSpacingX = canvasSize.x / myTimelinePreset->maxLength;
	for (size_t i = 0; i < myTimelinePreset->maxLength; i++)
	{
		if (i % 10 == 0)
		{
			drawlist->AddLine(canvasPos + ImVec2(lineSpacingX * i, 0), canvasPos + ImVec2(lineSpacingX * i, myTimelineSize.y), IM_COL32(255, 255, 255, 150));
			drawlist->AddLine(canvasPos + ImVec2(lineSpacingX * i, myTimelineSize.y), canvasPos + ImVec2(lineSpacingX * i, canvasSize.y), IM_COL32(60, 60, 60, 150));
		}
	}

	if (ImGui::IsMouseHoveringRect(canvasPos, canvasPos + myTimelineSize))
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	}

	if (ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] && myPlayingSequence == false && !myIsDraging)
	{
		if (ImGui::IsMouseHoveringRect(canvasPos, canvasPos + myTimelineSize))
		{
			myMovingMarker = true;
		}
	}
	else
	{
		myMovingMarker = false;
	}

	HandleTrackContent();

	if (myPreviewOn)
	{
		drawlist->AddTriangleFilled(myMPoint1, myMPoint2, myMPoint3, IM_COL32(255, 100, 100, 255));
	}
	else
	{
		drawlist->AddTriangleFilled(myMPoint1, myMPoint2, myMPoint3, IM_COL32(255, 255, 255, 255));
	}

	if (myMovingMarker)
	{
		myMPoint3 = ImVec2(ImGui::GetIO().MousePos.x, canvasPos.y + myTimelineSize.y);
		myMPoint1 = ImVec2(myMPoint3.x - myMarkerWidth, canvasPos.y + 5);
		myMPoint2 = ImVec2(myMPoint3.x + myMarkerWidth, canvasPos.y + 5);

		myMPoint3.x = glm::clamp(myMPoint3.x, myTimelinePos.x, myTimelinePos.x + myTimelineSize.x);

		drawlist->AddText(io.MousePos + ImVec2(5, -15), IM_COL32(255, 100, 100, 255), std::to_string(GetFrameFromPos(myMPoint3)).c_str());
	}

	drawlist->AddLine(myMPoint3, myMPoint3 + ImVec2(0, canvasSize.y - myTimelineSize.y), IM_COL32(255, 255, 255, 255));
	HandleTimelineInfo();

	ImGui::End();

}
