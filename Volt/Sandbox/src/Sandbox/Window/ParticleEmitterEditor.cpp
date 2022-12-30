#include "sbpch.h"
#include "ParticleEmitterEditor.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Volt/Particles/ParticleSystem.h"
#include <Volt/Asset/ParticlePreset.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Shader/Shader.h>

#include <Volt/Components/PostProcessComponents.h>

#include <Volt/Utility/UIUtility.h>

#include <random>


ParticleEmitterEditor::ParticleEmitterEditor()
	: EditorWindow("ParticleSystem Preset Editor")
{
	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	myPreviewScene = CreateRef<Volt::Scene>();

	//Particle Emitter
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::ParticleEmitterComponent& comp = entity.AddComponent<Volt::ParticleEmitterComponent>();
		comp.currentPreset = Volt::Asset::Null();
		myEmitterEntity = entity;
	}

	{
		auto ent = myPreviewScene->CreateEntity();
		ent.GetComponent<Volt::TagComponent>().tag = "Post Processing";
		ent.AddComponent<Volt::BloomComponent>();
		ent.AddComponent<Volt::FXAAComponent>();
		ent.AddComponent<Volt::HBAOComponent>();
	}

	myPreviewRenderer = CreateRef<Volt::SceneRenderer>(myPreviewScene);

	// Forward Extra
	{
		Volt::FramebufferSpecification spec{};

		spec.attachments =
		{
			{ Volt::ImageFormat::RGBA32F }, // Color
			{ Volt::ImageFormat::R32UI }, // ID
			{ Volt::ImageFormat::DEPTH32F }
		};

		spec.width = 1280;
		spec.height = 720;

		spec.existingImages =
		{
			{ 0, myPreviewRenderer->GetFinalFramebuffer()->GetColorAttachment(0) },
			{ 1, myPreviewRenderer->GetSelectionFramebuffer()->GetColorAttachment(2) },
		};

		spec.existingDepth = myPreviewRenderer->GetFinalObjectFramebuffer()->GetDepthAttachment();

		myForwardExtraPass.framebuffer = Volt::Framebuffer::Create(spec);
		myForwardExtraPass.debugName = "Forward Extra";
	}

	myPreviewRenderer->AddExternalPassCallback([this](Ref<Volt::Scene> scene, Ref<Volt::Camera> camera)
		{
			Volt::Renderer::BeginPass(myForwardExtraPass, camera);

			Volt::Renderer::SubmitSprite(gem::mat4{ 1.f }, { 1.f, 1.f, 1.f, 1.f }, myGridMaterial);
			Volt::Renderer::DispatchSpritesWithMaterial(myGridMaterial);

			Volt::Renderer::EndPass();
		});
}

void ParticleEmitterEditor::UpdateMainContent()
{
	const ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;
	if (ImGui::BeginTable("particleEditorMain", 2, tableFlags))
	{
		ImGui::TableSetupColumn("Viewport", 0, 250.f);
		ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		UpdateViewport();
		ImGui::TableNextColumn();
		UpdateProperties();

		ImGui::EndTable();
	}
}

void ParticleEmitterEditor::OpenAsset(Ref<Volt::Asset> asset)
{
	OpenParticleSystem(asset->path);
}


bool ParticleEmitterEditor::SavePreset(const std::filesystem::path& indata)
{
	if (indata == "None" || !myCurrentPreset)
		return false;

	Volt::AssetManager::Get().SaveAsset(myCurrentPreset);
	return true;
}

void ParticleEmitterEditor::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(ParticleEmitterEditor::OnRenderEvent));
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(ParticleEmitterEditor::OnUpdateEvent));

	myCameraController->OnEvent(e);
}

bool ParticleEmitterEditor::OnRenderEvent(Volt::AppRenderEvent& e)
{
	myPreviewRenderer->OnRenderEditor(myCameraController->GetCamera());

	return false;
}

bool ParticleEmitterEditor::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (!myCurrentPreset)
	{
		return false;
	}

	myPreviewScene->myParticleSystem->Update(e.GetTimestep());

	UpdateEmitter(e.GetTimestep());

	return false;
}

void ParticleEmitterEditor::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::BeginChild("viewport");
	myCameraController->SetIsControllable(ImGui::IsWindowHovered());

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (myViewportSize != (*(gem::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		myPreviewRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myPreviewScene->SetRenderSize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(myPreviewRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), viewportSize);
	ImGui::EndChild();
	ImGui::PopStyleVar(3);
}

void ParticleEmitterEditor::UpdateProperties()
{
	ImGui::BeginChild("properties");

	myPresets.clear();
	myPresets.emplace_back("None");
	for (auto& a : Volt::AssetManager::Get().GetAssetRegistry())
	{
		if (Volt::AssetManager::Get().GetAssetTypeFromHandle(a.second) == Volt::AssetType::ParticlePreset)
		{
			myPresets.emplace_back(a.first.string());
		}
	}

	static int currentPresetSelected = 0;
	if (UI::Combo("Emitter Preset", currentPresetSelected, myPresets, 500))
	{
		if (currentPresetSelected > 0)
		{
			OpenParticleSystem(myPresets[currentPresetSelected]);
		}
	}

	if (!myCurrentPreset)
	{
		ImGui::EndChild();
		return;
	}

	UI::PushId();

	//TABS
	static int tabs = 0;
	{
		if (ImGui::Button("Properties"))
		{
			tabs = 0;
		}

		ImGui::SameLine();

		if (ImGui::Button("Editor Settings"))
		{
			tabs = 1;
		}
	}

	if (tabs == 0)
	{
		ImGui::LabelText("", "Rendering");
		if (UI::BeginProperties("Render"))
		{
			EditorUtils::Property("Texture", myCurrentPreset->texture, Volt::AssetType::Texture);
			UI::Property("Shader", myCurrentPreset->shader);

			UI::EndProperties();
		}
		ImGui::Separator();

		ImGui::LabelText("", "Common Settings");
		if (UI::BeginProperties("Common"))
		{
			UI::Property("Looping", myCurrentPreset->isLooping);
			if (!myCurrentPreset->isLooping)
			{
				UI::Property("Duration", myCurrentPreset->emittionTime);
			}

			UI::Property("MinLifeTime", myCurrentPreset->minLifeTime);
			UI::Property("MaxLifeTime", myCurrentPreset->maxLifeTime);

			UI::EndProperties();
		}

		ImGui::LabelText("", "Emission");
		static int currentShapeSelected = 0;
		if (UI::Combo("Shape", currentShapeSelected, myShapes, 500))
		{
			myCurrentPreset->shape = currentShapeSelected;
		}

		if (UI::BeginProperties("Emitter"))
		{
			if (currentShapeSelected == 0)
			{
				UI::Property("SpawnOnEdge", myCurrentPreset->sphereSpawnOnEdge);
				UI::Property("Radius", myCurrentPreset->sphereRadius);
			}
			if (currentShapeSelected == 1)
			{
				UI::Property("SpawnOnEdge", myCurrentPreset->coneSpawnOnEdge);
				UI::Property("InnerRadius", myCurrentPreset->coneInnerRadius);
				UI::Property("OuterRadius", myCurrentPreset->coneOuterRadius);
			}
			myCurrentPreset->shape = currentShapeSelected;
			UI::Property("SpawnRate", myCurrentPreset->intensity, false, 0.f, 0.f, nullptr, "Amount of particles spawned each second");

			UI::EndProperties();
		}

		ImGui::LabelText("", "Velocity");
		if (UI::BeginProperties("Velocity"))
		{
			UI::Property("StartVelocity", myCurrentPreset->startVelocity);
			UI::Property("EndVelocity", myCurrentPreset->endVelocity);
			UI::Property("Gravity", myCurrentPreset->gravity);

			UI::EndProperties();
		}

		ImGui::LabelText("", "Size");
		if (UI::BeginProperties("size"))
		{
			UI::Property("StartSize", myCurrentPreset->startSize);
			UI::Property("EndSize", myCurrentPreset->endSize);

			UI::EndProperties();
		}

		ImGui::LabelText("", "Color");
		if (UI::BeginProperties("color"))
		{
			UI::PropertyColor("StartColor", myCurrentPreset->startColor);
			UI::PropertyColor("EndColor", myCurrentPreset->endColor);

			UI::EndProperties();
		}

		if (ImGui::Button("Play"))
		{
			PlayParticles();
		}
		if (ImGui::Button("Reset"))
		{
			OpenParticleSystem(myPresets[currentPresetSelected]);
		}
		if (ImGui::Button("Save Data"))
		{
			SavePreset(myPresets[currentPresetSelected]);
		}
	}

	if (tabs == 1)
	{
		ImGui::LabelText("", "Emitter Movement");
		if (UI::BeginProperties("Velocity"))
		{
			UI::Property("Move Emitter", myIsMoving);
			if (myIsMoving)
			{
				UI::Property("Move Length", myMoveLength);
				UI::Property("Move Speed", myMoveSpeed);
			}

			UI::EndProperties();
		}
	}

	UI::PopId();

	ImGui::EndChild();
}

void ParticleEmitterEditor::OpenParticleSystem(const std::filesystem::path& aPath)
{
	myCurrentPreset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(aPath);
	auto& emitterComp = myEmitterEntity.GetComponent<Volt::ParticleEmitterComponent>();

	emitterComp.preset = myCurrentPreset->handle;
	emitterComp.emittionTimer = myCurrentPreset->emittionTime;
	emitterComp.isLooping = myCurrentPreset->isLooping;
	emitterComp.numberOfAliveParticles = 0;
	myParticleSystemData = {};

	for (auto& p : emitterComp.particles)
	{
		p.dead = true;
	}
}

void ParticleEmitterEditor::PlayParticles()
{
	auto& emitterComp = myEmitterEntity.GetComponent<Volt::ParticleEmitterComponent>();

	emitterComp.preset = myCurrentPreset->handle;
	emitterComp.emittionTimer = myCurrentPreset->emittionTime;
	emitterComp.isLooping = myCurrentPreset->isLooping;
	emitterComp.numberOfAliveParticles = 0;
	emitterComp.pressedPlay = true;
	myParticleSystemData = {};

	for (auto& p : emitterComp.particles)
	{
		p.dead = true;
	}
}

void ParticleEmitterEditor::UpdateEmitter(float aDeltaTime)
{
	if (myIsMoving)
	{
		gem::vec3 entPos = myEmitterEntity.GetPosition();

		if (entPos.x > myMoveLength || entPos.x < - myMoveLength)
		{
			myMoveSpeed *= -1;
		}
		myEmitterEntity.SetPosition(entPos + myEmitterEntity.GetRight() * myMoveSpeed * aDeltaTime);
	}
	else
	{
		myEmitterEntity.SetPosition(0);
	}
}
