#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Rendering/RenderPass.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Asset/Asset.h>

#include <gem/gem.h>

namespace Volt
{
	class Framebuffer;
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

private:
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

	void RecursiveGetParent(const std::string& name, TempJoint& joint, TempJoint* outJoint);

	void RecursiveRenderJoint(TempJoint& joint);

	Ref<Volt::Scene> myScene;
	Scope<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::Material> myGridMaterial;

	Volt::RenderPass myForwardExtraPass;

	Ref<Volt::AnimatedCharacter> myCurrentCharacter;
	Ref<EditorCameraController> myCameraController;

	Volt::Entity myCharacterEntity;

	gem::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	gem::vec2 myViewportSize = { 1280.f, 720.f };

	const float myButtonSize = 22.f;

	bool myIsPlayingAnim = false;

	Volt::AssetHandle mySkinHandle = Volt::Asset::Null();
	Volt::AssetHandle mySkeletonHandle = Volt::Asset::Null();;

	NewCharacterData myNewCharacterData{};
};