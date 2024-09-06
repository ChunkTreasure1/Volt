#include "sbpch.h"
#include "Window/ParticleEmitterEditor.h"

#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Volt/Particles/ParticleSystem.h"
#include <Volt/Asset/ParticlePreset.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Volt/Components/LightComponents.h>
#include <Volt/Components/RenderingComponents.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Project/ProjectManager.h>

#include <AssetSystem/AssetManager.h>

#include <WindowModule/Events/WindowEvents.h>

#include <random>

ParticleEmitterEditor::ParticleEmitterEditor()
	: EditorWindow("Particle Editor")
{
	m_windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	RegisterListener<Volt::WindowRenderEvent>(VT_BIND_EVENT_FN(ParticleEmitterEditor::OnRenderEvent));
	RegisterListener<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(ParticleEmitterEditor::OnUpdateEvent));

	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);
	myPreviewScene = Volt::Scene::CreateDefaultScene("Particle Editor", false);
	myReferenceModel = myPreviewScene->CreateEntity("Reference Entity");
	myReferenceModel.AddComponent<Volt::MeshComponent>();

	myLightEntity = myPreviewScene->GetAllEntitiesWith<Volt::DirectionalLightComponent>()[0];
	auto& tempComp = myLightEntity.GetComponent<Volt::DirectionalLightComponent>();
	tempComp.castShadows = false;
	tempComp.softShadows = false;
	//Particle Emitter
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::ParticleEmitterComponent& comp = entity.AddComponent<Volt::ParticleEmitterComponent>();
		comp.currentPreset = Volt::Asset::Null();
		myEmitterEntity = entity;
	}

	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Particle System Editor";
		spec.scene = myPreviewScene;

		//Volt::SceneRendererSettings settings{};
		//settings.enableGrid = true;

		myPreviewRenderer = CreateRef<Volt::SceneRenderer>(spec);
	}
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
	int i = 0;
	for (const auto& [handle, metadata] : Volt::AssetManager::Get().GetAssetRegistry())
	{
		if (Volt::AssetManager::Get().GetAssetTypeFromHandle(handle) == AssetTypes::ParticlePreset)
		{
			i++;
			if (asset->handle == handle)
			{
				currentPresetSelected = i;
				Volt::AssetManager::Get().ReloadAsset(handle);
				break;
			}
		}
	}


	OpenParticleSystem(asset->handle);
}

bool ParticleEmitterEditor::SavePreset(const std::filesystem::path& indata)
{
	if (indata == "None" || !myCurrentPreset)
	{
		UI::Notify(NotificationType::Error, "ParticlePreset save Failed", "Invalid preset");
		return false;
	}

	const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myCurrentPreset->handle);
	if (!FileSystem::IsWriteable(Volt::ProjectManager::GetRootDirectory() / metadata.filePath))
	{
		UI::Notify(NotificationType::Error, "ParticlePreset save Failed", "Make sure file is writable");
		return false;
	}
	UI::Notify(NotificationType::Success, "ParticlePreset Saved ", "");
	Volt::AssetManager::Get().SaveAsset(myCurrentPreset);
	return true;
}

bool ParticleEmitterEditor::OnRenderEvent(Volt::WindowRenderEvent& e)
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
	myPreviewScene->GetParticleSystem().Update(myPreviewScene->GetRegistry(), myPreviewScene, e.GetTimestep());
	UpdateEmitter(e.GetTimestep());
	return false;
}

void ParticleEmitterEditor::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::BeginChild("viewport");

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (myViewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		myPreviewRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myPreviewScene->SetRenderSize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(myPreviewRenderer->GetFinalImage()), viewportSize);
	ImGui::EndChild();
	ImGui::PopStyleVar(3);
}

void ParticleEmitterEditor::UpdateProperties()
{
	if (DrawEditorPanel())
	{
		DrawPropertiesPanel();
	}
}

void ParticleEmitterEditor::OpenParticleSystem(const Volt::AssetHandle handle)
{
	myCurrentPreset = Volt::AssetManager::GetAsset<Volt::ParticlePreset>(handle);
	if (!myCurrentPreset)
		return;

	auto& emitterComp = myEmitterEntity.GetComponent<Volt::ParticleEmitterComponent>();
	emitterComp.preset = myCurrentPreset->handle;
	emitterComp.emissionTimer = myCurrentPreset->emittionTime;
	emitterComp.burstTimer = 0;
	emitterComp.isLooping = myCurrentPreset->isLooping;

	/*emitterComp.numberOfAliveParticles = 0;
	for (auto& p : emitterComp.particles)
	{
		p.dead = true;
	}*/
}

void ParticleEmitterEditor::PlayParticles()
{
	auto& emitterComp = myEmitterEntity.GetComponent<Volt::ParticleEmitterComponent>();

	emitterComp.preset = myCurrentPreset->handle;
	emitterComp.burstTimer = 0;
	emitterComp.emissionTimer = myCurrentPreset->emittionTime;
	emitterComp.isLooping = myCurrentPreset->isLooping;

	//emitterComp.numberOfAliveParticles = 0;
	//emitterComp.pressedPlay = true;
	/*for (auto& p : emitterComp.particles)
	{
		p.dead = true;
	}*/
}

bool ParticleEmitterEditor::DrawEditorPanel()
{
	ImGui::BeginChild("editor", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.3f }, true);
	UI::Header("Editor");
	ImGui::Separator();
	{
		//#TODO_Ivar: Fix this mess
		//myPresets.clear();
		//myPresets.emplace_back("None");
		//for (const auto& handle : Volt::AssetManager::GetAllAssetsOfType(AssetType::ParticlePreset))
		//{
		//	const auto& meta = Volt::AssetManager::GetMetadataFromHandle(handle);
		//	myPresets.emplace_back(meta.filePath.string());
		//}

		//if (UI::Combo("Emitter Preset", currentPresetSelected, myPresets, ImGui::GetContentRegionAvail().x - 103))
		//{
		//	if (currentPresetSelected >= 0)
		//	{
		//		OpenParticleSystem(myPresets[currentPresetSelected]);
		//	}
		//}

		if (ImGui::Button("Play"))
		{
			if (currentPresetSelected != 0)
				PlayParticles();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			//if (currentPresetSelected != 0)
				//OpenParticleSystem(myPresets[currentPresetSelected]);
		}
		ImGui::SameLine();
		ImGui::Dummy({ ((ImGui::GetContentRegionAvail().x - 84) > 0) ? (ImGui::GetContentRegionAvail().x - 84) : 0, 0 });
		ImGui::SameLine();
		if (ImGui::Button("Save Data") != 0)
		{
			SavePreset(myPresets[currentPresetSelected]);
		}

		ImGui::BeginChild("editor settings", ImGui::GetContentRegionAvail(), false);


		if (ImGui::CollapsingHeader("Scene"))
		{
			// Reference model begin 
			ImGui::Checkbox("##referenceModelVisablity", &myReferenceModel.GetComponent<Volt::TransformComponent>().visible);
			ImGui::SameLine();
			if (ImGui::TreeNode("Reference Model"))
			{
				ImGui::Separator();
				if (UI::BeginProperties("emitterScene"))
				{
					static glm::vec3 modelPos{ 0 };
					static glm::vec3 modelRot{ 0 };
					static glm::vec3 modelScale{ 1 };

					UI::PropertyAxisColor("Position", modelPos);
					UI::PropertyAxisColor("Rotation", modelRot);
					UI::PropertyAxisColor("Scale", modelScale);
					EditorUtils::Property("Mesh", myReferenceModel.GetComponent<Volt::MeshComponent>().handle, AssetTypes::Mesh);
					//EditorUtils::Property("Material", myReferenceModel.GetComponent<Volt::MeshComponent>().material, AssetType::Material);

					myReferenceModel.SetPosition(modelPos);
					myReferenceModel.SetRotation(modelRot);
					myReferenceModel.SetScale(modelScale);
					UI::EndProperties();
				}
				ImGui::TreePop();
			}
			// Reference model end

			if (UI::BeginProperties("emitterSceneSkybox"))
			{
				static glm::vec3 lightRot{ 0 };
				static glm::vec3 lightColor{ 0 };
				UI::PropertyAxisColor("Light Rotation", lightRot);
				myLightEntity.SetRotation(lightRot);

				static float cameraSpeed = 100;
				cameraSpeed = myCameraController->GetTranslationSpeed();
				UI::Property("Camera speed", cameraSpeed);
				myCameraController->SetTranslationSpeed(cameraSpeed);

				EditorUtils::Property("Skybox", myPreviewScene->GetAllEntitiesWith<Volt::SkylightComponent>()[0].GetComponent<Volt::SkylightComponent>().environmentHandle, AssetTypes::Texture);
				UI::EndProperties();
			}
		}

		ImGui::Checkbox("##movingCheckb", &myIsMoving);
		ImGui::SameLine();
		if (ImGui::CollapsingHeader("Emitter Movement##emitterEditorSettingsMovementTab"))
		{
			if (UI::BeginProperties("Velocity"))
			{
				UI::Property("Length", myMoveLength);
				UI::Property("Speed", myMoveSpeed);
			}
			UI::EndProperties();
		}
		ImGui::EndChild();
	} ImGui::EndChild();
	if (currentPresetSelected != 0)
		return true;
	return false;
}

void ParticleEmitterEditor::DrawPropertiesPanel()
{
	ImGui::BeginChild("##properties", ImGui::GetContentRegionAvail(), true);
	{
		UI::Header("Properties");
		ImGui::Separator();
		ImGui::BeginChild("##propScroll");
		{
			if (ImGui::CollapsingHeader("Rendering##emitterRenderMode"))
			{
				static int selectedRenderMode = (int)myCurrentPreset->type;
				Vector<std::string> emitterModes = { "Mesh","Particle" };

				if (UI::Combo("Emitter Type", selectedRenderMode, emitterModes))
				{
					myCurrentPreset->type = (Volt::ParticlePreset::eType)selectedRenderMode;
				}

				if (UI::BeginProperties("Render"))
				{
					switch (myCurrentPreset->type)
					{
						case Volt::ParticlePreset::eType::PARTICLE:
							EditorUtils::Property("Texture", myCurrentPreset->texture, AssetTypes::Texture);
							EditorUtils::Property("Material", myCurrentPreset->material, AssetTypes::Material);
							break;
						case Volt::ParticlePreset::eType::MESH:
							EditorUtils::Property("Mesh", myCurrentPreset->mesh, AssetTypes::Mesh);
							EditorUtils::Property("Material", myCurrentPreset->material, AssetTypes::Material);
							break;
						default:
							break;
					}
					UI::EndProperties();
				}
			}
			if (ImGui::CollapsingHeader("Emission"))
			{
				ImGui::LabelText("##emitterLabelLifeTime", "Life Time");
				ImGui::Separator();
				if (UI::BeginProperties("Life"))
				{
					UI::Property("Looping", myCurrentPreset->isLooping);
					if (!myCurrentPreset->isLooping)
					{
						ImGui::SameLine();
						//ImGui::TextUnformatted("Duration");
						//ImGui::SameLine();
						ImGui::PushItemWidth(ImGui::GetColumnWidth());
						ImGui::DragFloat("##duration", &myCurrentPreset->emittionTime, 0.1f);
						ImGui::PopItemWidth();
					}

					static glm::vec2 lifespan{ myCurrentPreset->minLifeTime, myCurrentPreset->maxLifeTime };
					lifespan = { myCurrentPreset->minLifeTime ,myCurrentPreset->maxLifeTime };

					UI::Property("Particle Lifespan", lifespan, 0.f, 0.f, "x = min life time, y = max life time");
					myCurrentPreset->minLifeTime = lifespan.x;
					myCurrentPreset->maxLifeTime = (lifespan.x > lifespan.y) ? lifespan.x : lifespan.y;
					//UI::Property("MaxLifeTime", );

					UI::EndProperties();
				}

				ImGui::LabelText("##emitterSettingSpawningLabel", "Spawning");
				ImGui::Separator();
				if (UI::BeginProperties("Emitter"))
				{
					UI::Property("Intensity", myCurrentPreset->intensity, 0.f, 0.f, "Amount of particles spawned each second");
					UI::EndProperties();
				}
				ImGui::Checkbox("##burstCheckbox", &myCurrentPreset->isBurst);
				ImGui::SameLine();
				if (ImGui::TreeNode("Burst Emission##emitterSettingBurstTreeNode"))
				{
					if (UI::BeginProperties())
					{
						UI::PropertyDragFloat("Burst length", myCurrentPreset->burstLength, 0.1f);
						UI::PropertyDragFloat("Burst interval", myCurrentPreset->burstInterval, 0.1f);
						UI::EndProperties();
						ImGui::Separator();
					}
					ImGui::TreePop();
				}
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
					UI::EndProperties();
				}
			}
			if (ImGui::CollapsingHeader("Update"))
			{
				ImGui::LabelText("", "Movement");
				ImGui::Separator();
				if (UI::BeginProperties("Velocity"))
				{
					UI::Property("StartVelocity", myCurrentPreset->startVelocity);
					UI::Property("EndVelocity", myCurrentPreset->endVelocity);
					UI::Property("Gravity", myCurrentPreset->gravity);

					UI::EndProperties();
				}

				DrawElementSize();
				DrawElementColor();
			}
		} ImGui::EndChild();
	}ImGui::EndChild();
}

void ParticleEmitterEditor::DrawElementColor()
{
	if (myCurrentPreset->type != Volt::ParticlePreset::eType::PARTICLE)
		return;

	if (ImGui::Button("-##colorRemove"))
	{
		if (myCurrentPreset->colors.size() > 1)
		{
			myCurrentPreset->colors.erase(myCurrentPreset->colors.begin() + myCurrentPreset->colors.size() - 1);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("+##colorAdd"))
	{
		myCurrentPreset->colors.push_back({ 0,0,0,0 });
	}
	ImGui::SameLine();
	if (ImGui::TreeNode("Colors##PPColorTreeNode"))
	{
		ImGui::Separator();
		ImGui::BeginChild("##color settings", { ImGui::GetContentRegionAvail().x, 100.0f }, false);
		if (UI::BeginProperties("color"))
		{
			for (int i = 0; i < myCurrentPreset->colors.size(); i++)
			{
				UI::PropertyColor(std::to_string(i + 1) + ": ", myCurrentPreset->colors[i]);
			}
			UI::EndProperties();
		}ImGui::EndChild();
		ImGui::TreePop();
	}
}

void ParticleEmitterEditor::DrawElementSize()
{
	if (ImGui::Button("-##sizeRemove"))
	{
		if (myCurrentPreset->sizes.size() > 1)
		{
			myCurrentPreset->sizes.erase(myCurrentPreset->sizes.begin() + myCurrentPreset->sizes.size() - 1);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("+##sizeAdd"))
	{
		myCurrentPreset->sizes.push_back({ 0,0,0 });
	}
	ImGui::SameLine();
	if (ImGui::TreeNode("Size##SizeTreeNode"))
	{
		ImGui::Separator();
		ImGui::BeginChild("##size settings", { ImGui::GetContentRegionAvail().x, 100.0f }, false);
		if (UI::BeginProperties("size"))
		{
			for (int i = 0; i < myCurrentPreset->sizes.size(); i++)
			{
				UI::Property(std::to_string(i + 1) + ": ", myCurrentPreset->sizes[i]);
			}
			UI::EndProperties();
		}ImGui::EndChild();
		ImGui::TreePop();
	}
}

void ParticleEmitterEditor::UpdateEmitter(float aDeltaTime)
{
	if (myIsMoving)
	{
		glm::vec3 entPos = myEmitterEntity.GetLocalPosition();

		if (entPos.x > myMoveLength || entPos.x < -myMoveLength)
		{
			myMoveSpeed *= -1;
		}
		myEmitterEntity.SetLocalPosition(entPos + myEmitterEntity.GetLocalRight() * myMoveSpeed * aDeltaTime);
	}
	else
	{
		myEmitterEntity.SetLocalPosition(0);
	}
}
