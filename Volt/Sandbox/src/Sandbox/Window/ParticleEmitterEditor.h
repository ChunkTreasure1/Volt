#pragma once
#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Core/Base.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Rendering/RenderPass.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Particles/Particle.h>

#include "Volt/Rendering/SceneRenderer.h"

#include <vector>

namespace Volt
{
	class Shader;
	class ParticlePreset;
	class Material;
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
		std::vector<Volt::Particle> particles;
	};

	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);

	void UpdateEmitter(float aDeltaTime);
	void UpdateViewport();
	void UpdateProperties();

	void OpenParticleSystem(const std::filesystem::path& aPath);
	void PlayParticles();

	ParticleSystemData myParticleSystemData;

	Volt::Entity myEmitterEntity;

	Volt::RenderPass myForwardExtraPass;

	std::vector<std::string> myPresets;
	std::vector<std::string> myShapes{"Sphere", "Cone"};

	gem::vec2 myPerspectiveBounds[2] = { { 0.f, 0.f }, { 0.f, 0.f } };
	gem::vec2 myViewportSize = { 1280.f, 720.f };
	
	Ref<Volt::ParticlePreset> myCurrentPreset;

	Ref<Volt::Scene> myPreviewScene;
	Ref<Volt::SceneRenderer> myPreviewRenderer;
	Ref<Volt::Material> myGridMaterial;

	Ref<EditorCameraController> myCameraController;

	//Editor Settings
	bool myIsMoving = false;
	float myMoveLength = 0.f;
	float myMoveSpeed = 0.f;
};