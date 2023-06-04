#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Asset/Asset.h>

#include <GEM/gem.h>

namespace Volt
{
	class Shader;
	class AnimatedCharacter;
	class ComputePipeline;
	class Scene;
	class SceneRenderer;
	class Material;
}

class EditorCameraController;
class CharacterEditorPanel : public EditorWindow
{
public:
	CharacterEditorPanel();
	~CharacterEditorPanel() override = default;

	void UpdateMainContent() override;
	void UpdateContent() override;
	void OnEvent(Volt::Event& e) override;
	void OpenAsset(Ref<Volt::Asset> asset) override;

	void OnOpen() override;
	void OnClose() override;

private:
	struct AddAnimEventData
	{
		uint32_t frame;
		std::string name;
	};

	struct TempJoint
	{
		std::string name;
		std::vector<TempJoint> childJoints;
	};

	struct TempSkeleton
	{
		std::vector<TempJoint> joints;
	};

	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);

	void UpdateToolbar();
	void UpdateViewport();
	void UpdateProperties();
	void UpdateAnimations();
	void UpdateSkeletonView();
	void UpdateAnimationTimelinePanel();
	void UpdateJointAttachmentViewPanel();

	void AddAnimationEventModal();
	void AddJointAttachmentPopup();

	void RecursiveGetParent(const std::string& name, TempJoint& joint, TempJoint*& outJoint);
	void RecursiveRenderJoint(TempJoint& joint);

	Ref<Volt::Scene> myScene;
	Scope<Volt::SceneRenderer> mySceneRenderer;

	int32_t mySelectedAnimation = -1;
	int32_t mySelectedKeyframe = -1;

	Ref<Volt::AnimatedCharacter> myCurrentCharacter;
	Ref<EditorCameraController> myCameraController;

	Volt::Entity myCharacterEntity;

	gem::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	gem::vec2 myViewportSize = { 1280.f, 720.f };

	const float myButtonSize = 22.f;

	bool myIsPlayingAnim = false;
	
	bool myActivateJointSearch = false;
	std::string myJointSearchQuery;

	std::vector<Volt::Entity> myJointAttachmentEntities;

	Volt::AssetHandle mySkinHandle = Volt::Asset::Null();
	Volt::AssetHandle mySkeletonHandle = Volt::Asset::Null();;

	AddAnimEventData myAddAnimEventData{};
	NewCharacterData myNewCharacterData{};
};
