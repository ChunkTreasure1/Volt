#pragma once
#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Core/Base.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Events/ApplicationEvents.h>
#include <Volt/Particles/Particle.h>



namespace Volt
{
	class ParticlePreset;
	class Material;
	class SceneRenderer;
	class WindowRenderEvent;
}

class EditorCameraController;
class ParticleEmitterEditor : public EditorWindow
{
public:
	ParticleEmitterEditor();
	void UpdateMainContent() override;
	void OpenAsset(Ref<Volt::Asset> asset) override;
	bool SavePreset(const std::filesystem::path& indata);

	void OnEvent(Volt::Event& e) override;

private:
	struct ParticleSystemData
	{
		float emittionTimer = 0;
		int numberOfAliveParticles = 0;
		Vector<Volt::Particle> particles;
	};

	bool OnRenderEvent(Volt::WindowRenderEvent& e);
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);

	void UpdateEmitter(float aDeltaTime);
	void UpdateViewport();
	void UpdateProperties();

	void OpenParticleSystem(const Volt::AssetHandle handle);
	void PlayParticles();
		
	bool DrawEditorPanel();
	void DrawPropertiesPanel();

	void DrawElementColor();
	void DrawElementSize();


	//ParticleSystemData myParticleSystemData;

	Volt::Entity myEmitterEntity;

	Vector<std::string> myPresets;
	Vector<std::string> myShapes{ "Sphere", "Cone" };

	glm::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	glm::vec2 myViewportSize = { 1280.f, 720.f };

	Ref<Volt::ParticlePreset> myCurrentPreset;

	Ref<Volt::Scene> myPreviewScene;
	Ref<Volt::SceneRenderer> myPreviewRenderer;
	Ref<Volt::Material> myGridMaterial;

	Ref<EditorCameraController> myCameraController;

	//Editor Settings
	bool myIsMoving = false;
	float myMoveLength = 500.f;
	float myMoveSpeed = 250.f;
	int currentPresetSelected = 0;

	Volt::Entity myReferenceModel;
	Volt::Entity myLightEntity;
};
