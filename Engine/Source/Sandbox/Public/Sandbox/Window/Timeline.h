#pragma once
#include "Sandbox/Window/EditorWindow.h"
#include <Volt/Scene/Scene.h>
#include "Volt/Asset/TimelinePreset.h"

#include <InputModule/Events/KeyboardEvents.h>

#include <EventSystem/ApplicationEvents.h>

#include <Sandbox/Camera/EditorCameraController.h>

class Timeline : public EditorWindow
{
public:
	Timeline(Ref<Volt::Scene>& aScene, EditorCameraController* editorCamera);
	~Timeline() { mySelectedKeyframe = nullptr; mySelectedClip = nullptr; }
	void UpdateMainContent() override;
	void UpdateContent() override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnEvent(Volt::Event& e);

private:

	int GetFrameFromPos(ImVec2 pos);

	void SaveAsset();

	void PlaySequence();

	void OnPlay();
	void OnStop();

	void HandleTimelineInfo();
	void HandleTrackContent();

	void HandleSelectedKeyframe();
	void HandleSelectedClip();

	void SortTrack(Volt::Track& track);

	bool CameraQuickshotKeyframe(Volt::KeyPressedEvent& e);
	void AddKeyframe();
	void AddClip(Volt::EntityID entityId);

	void PreviewTimeline();

	void DrawEntityTracks(ImDrawList& drawlist, int trackIndex);
	void DrawKeyframesOnTrack(int trackIndex);
	void DrawClipsOnTrack(int trackIndex);

	void UpdateTimelineTracks();
	void UpdateTimeLine();

	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	
	float myDeltaTime = 0.f;

	Ref<Volt::Scene>& myCurrentScene;
	EditorCameraController* myEditorCamera;

	Ref<Volt::TimelinePreset> myTimelinePreset;
	
	float* mySelectedClipEdge = nullptr;
	Volt::Keyframe* mySelectedKeyframe = nullptr;
	Volt::Clip* mySelectedClip = nullptr;
	Volt::Track* mySelectedTrack = nullptr;

	ImVec2 myTrackWindowSize;
	ImVec2 myTrackWindowPos;

	//Time bar Values
	float myMarkerWidth = 7.f;
	ImVec2 myTimelineSize;
	ImVec2 myTimelinePos;

	bool myIsDraging = false;
	bool myIsStretching = false;

	bool myMovingMarker = false;

	bool myPlayingSequence = false;
	bool myPreviewOn = false;

	ImVec2 myIndividualTrackSize;
	ImVec2 mySelectedTrackMinPos;
	ImVec2 mySelectedTrackMaxPos;

	//Marker Points
	ImVec2 myMPoint1;
	ImVec2 myMPoint2;
	ImVec2 myMPoint3;
};
