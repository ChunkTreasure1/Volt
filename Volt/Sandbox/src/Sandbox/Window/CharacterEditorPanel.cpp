#include "sbpch.h"
#include "CharacterEditorPanel.h"

#include "Sandbox/Window/EditorIconLibrary.h"
#include "Sandbox/Camera/EditorCameraController.h"

#include <Volt/Animation/AnimationManager.h>

#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/Entity.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Utility/UIUtility.h>

CharacterEditorPanel::CharacterEditorPanel()
	: EditorWindow("Character Editor", true)
{
	myWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	myScene = CreateRef<Volt::Scene>();
	mySceneRenderer = CreateScope<Volt::SceneRenderer>(myScene);

	mySceneRenderer->AddExternalPassCallback([this](Ref<Volt::Scene> scene, Ref<Volt::Camera> camera)
		{
			Volt::Renderer::BeginPass(myForwardExtraPass, camera);

			Volt::Renderer::SubmitSprite(gem::mat4{ 1.f }, { 1.f, 1.f, 1.f, 1.f });
			Volt::Renderer::DispatchSpritesWithShader(Volt::ShaderRegistry::Get("Grid"));

			Volt::Renderer::EndPass();
		});

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
			{ 0, mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0) },
			{ 1, mySceneRenderer->GetSelectionFramebuffer()->GetColorAttachment(2) },
		};

		spec.existingDepth = mySceneRenderer->GetFinalObjectFramebuffer()->GetDepthAttachment();

		myForwardExtraPass.framebuffer = Volt::Framebuffer::Create(spec);
		myForwardExtraPass.debugName = "Forward Extra";
	}

	// Skylight
	{
		auto entity = myScene->CreateEntity();
		Volt::SkylightComponent& comp = entity.AddComponent<Volt::SkylightComponent>();
		comp.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Assets/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
	}

	// Directional light
	{
		auto entity = myScene->CreateEntity();
		Volt::DirectionalLightComponent& comp = entity.AddComponent<Volt::DirectionalLightComponent>();
		comp.castShadows = false;
		comp.intensity = 3.f;

		entity.SetRotation(gem::quat(gem::radians(gem::vec3{ 70.f, 0.f, 100.f })));
	}

	// Character entityw
	{
		myCharacterEntity = myScene->CreateEntity();
		myCharacterEntity.AddComponent<Volt::AnimatedCharacterComponent>();
	}
}

void CharacterEditorPanel::UpdateMainContent()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Character"))
			{
				myNewCharacterData = {};
				UI::OpenModal("New Character##NewCharacterEditor");
			}

			if (ImGui::MenuItem("Open"))
			{
				const std::filesystem::path path = FileSystem::OpenFile("Animated Character (*.vtchr)\0*.vtchr\0");
				if (!path.empty() && FileSystem::Exists(path))
				{
					myCurrentCharacter = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(path);
					if (myCurrentCharacter)
					{
						mySkinHandle = myCurrentCharacter->GetSkin()->handle;
						mySkeletonHandle = myCurrentCharacter->GetSkeleton()->handle;
					}
					else
					{
						mySkinHandle = Volt::Asset::Null();
						mySkeletonHandle = Volt::Asset::Null();
					}
				}
			}

			if (ImGui::MenuItem("Save"))
			{
				if (myCurrentCharacter)
				{
					Volt::AssetManager::Get().SaveAsset(myCurrentCharacter);
					UI::Notify(NotificationType::Success, "Saved character!", std::format("Character {0} successfully saved!", myCurrentCharacter->path.stem().string()));
				}
			}

			if (ImGui::MenuItem("Save As"))
			{
				if (myCurrentCharacter)
				{

				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void CharacterEditorPanel::UpdateContent()
{
	UpdateProperties();
	UpdateAnimations();
	UpdateViewport();
	UpdateToolbar();
	UpdateSkeletonView();

	EditorUtils::NewCharacterModal("New Character##NewCharacterEditor", myCurrentCharacter, myNewCharacterData);
}

void CharacterEditorPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(CharacterEditorPanel::OnRenderEvent));
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(CharacterEditorPanel::OnUpdateEvent));

	myCameraController->OnEvent(e);
}

void CharacterEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (myCurrentCharacter)
	{
		Volt::AssetManager::Get().SaveAsset(asset);
	}

	myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>().animatedCharacter = asset->handle;

	myCurrentCharacter = std::reinterpret_pointer_cast<Volt::AnimatedCharacter>(asset);
	if (myCurrentCharacter && myCurrentCharacter->IsValid())
	{
		if (myCurrentCharacter->GetSkin())
		{
			mySkinHandle = myCurrentCharacter->GetSkin()->handle;
		}
		if (myCurrentCharacter->GetSkeleton())
		{
			mySkeletonHandle = myCurrentCharacter->GetSkeleton()->handle;
		}
	}
	else
	{
		mySkinHandle = Volt::Asset::Null();
		mySkeletonHandle = Volt::Asset::Null();
	}
}

bool CharacterEditorPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	mySceneRenderer->OnRenderEditor(myCameraController->GetCamera());
	return false;
}

bool CharacterEditorPanel::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (myIsPlayingAnim && myCurrentCharacter && myCurrentCharacter->IsValid())
	{
		Volt::AnimationManager::Update(e.GetTimestep());

		auto& comp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

		if (comp.isLooping && comp.currentStartTime + myCurrentCharacter->GetAnimationDuration(comp.currentAnimation) <= Volt::AnimationManager::globalClock)
		{
			comp.currentStartTime = Volt::AnimationManager::globalClock;
		}
	}

	myScene->UpdateEditor(e.GetTimestep());

	return false;
}

void CharacterEditorPanel::UpdateToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbarCharEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (UI::ImageButton("##Save", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
	{
		if (myCurrentCharacter)
		{
			Volt::AssetManager::Get().SaveAsset(myCurrentCharacter);
			UI::Notify(NotificationType::Success, "Saved Character!", std::format("Saved character {0} to file!", myCurrentCharacter->path.stem().string()));
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##Load", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Open)), { myButtonSize, myButtonSize }))
	{
		const std::filesystem::path characterPath = FileSystem::OpenFile("Animated Character (*.vtchr)\0*.vtchr\0");
		if (!characterPath.empty() && FileSystem::Exists(characterPath))
		{
			myCurrentCharacter = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(characterPath);
			if (myCurrentCharacter)
			{
				mySkinHandle = myCurrentCharacter->GetSkin()->handle;
				mySkeletonHandle = myCurrentCharacter->GetSkeleton()->handle;
			}
			else
			{
				mySkinHandle = Volt::Asset::Null();
				mySkeletonHandle = Volt::Asset::Null();
			}
		}
	}


	ImGui::PopStyleVar(2);
	ImGui::End();
}

void CharacterEditorPanel::UpdateViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

	ImGui::Begin("Viewport##charEdit");

	myCameraController->SetIsControllable(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows));

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (myViewportSize != (*(gem::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		mySceneRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), viewportSize);
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void CharacterEditorPanel::UpdateProperties()
{
	ImGui::Begin("Properties##charEdit");
	if (UI::BeginProperties("CharProperties"))
	{
		auto& charComp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

		UI::Property("Is Looping", charComp.isLooping);

		if (EditorUtils::Property("Skin", mySkinHandle, Volt::AssetType::Mesh))
		{
			Ref<Volt::Mesh> skin = Volt::AssetManager::GetAsset<Volt::Mesh>(mySkinHandle);
			if (skin && skin->IsValid())
			{
				myCurrentCharacter->SetSkin(skin);
			}
		}

		if (EditorUtils::Property("Skeleton", mySkeletonHandle, Volt::AssetType::Skeleton))
		{
			Ref<Volt::Skeleton> skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(mySkinHandle);
			if (skeleton && skeleton->IsValid())
			{
				myCurrentCharacter->SetSkeleton(skeleton);
			}
		}


		UI::EndProperties();
	}
	ImGui::End();
}

void CharacterEditorPanel::UpdateAnimations()
{
	const float buttonSize = 22.f;

	ImGui::Begin("Animations##charEdit");

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	ImGui::BeginChild("AnimBar", { ImGui::GetContentRegionAvail().x, myButtonSize + 5.f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

		// Adding
		{
			Volt::AssetHandle addHandle = Volt::Asset::Null();
			if (UI::ImageButton("##Add", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Add)), { buttonSize, buttonSize }))
			{
				ImGui::OpenPopup("animAddPopup");
			}

			if (EditorUtils::AssetBrowserPopupField("animAddPopup", addHandle, Volt::AssetType::Animation))
			{
				if (addHandle != Volt::Asset::Null())
				{
					myCurrentCharacter->SetAnimation((uint32_t)myCurrentCharacter->GetAnimationCount(), Volt::AssetManager::GetAsset<Volt::Animation>(addHandle));
				}
			}
		}

		ImGui::SameLine();

		// Play
		{
			Ref<Volt::Texture2D> icon = myIsPlayingAnim ? EditorIconLibrary::GetIcon(EditorIcon::Stop) : EditorIconLibrary::GetIcon(EditorIcon::Play);
			if (UI::ImageButton("##Play", UI::GetTextureID(icon), { buttonSize, buttonSize }))
			{
				myIsPlayingAnim = !myIsPlayingAnim;
				if (myIsPlayingAnim)
				{
					myScene->OnRuntimeStart();
				}
				else
				{
					myScene->OnRuntimeEnd();
				}
			}
		}

		ImGui::PopStyleVar(2);
	}
	ImGui::EndChild();

	ImGui::Separator();

	ImGui::BeginChild("Content", ImGui::GetContentRegionAvail());
	{
		if (ImGui::BeginTable("AnimTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn("Animation", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (const auto& [index, anim] : myCurrentCharacter->GetAnimations())
			{
				ImGui::TableNextColumn();
				ImGui::Text("%d", index);

				ImGui::TableNextColumn();

				std::string animName;
				if (anim && anim->IsValid())
				{
					animName = anim->path.stem().string();
				}
				else
				{
					"Null";
				}

				std::string popupName = "animPopup" + std::to_string(index);

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				UI::InputText("", animName, ImGuiInputTextFlags_ReadOnly);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					ImGui::OpenPopup(popupName.c_str());
				}
				ImGui::PopItemWidth();

				std::string rightClickId = "animRightClick" + std::to_string(index);
				if (ImGui::BeginPopupContextItem(rightClickId.c_str(), ImGuiPopupFlags_MouseButtonRight))
				{
					if (ImGui::MenuItem("Play") && anim && anim->IsValid())
					{
						if (!myIsPlayingAnim)
						{
							myScene->OnRuntimeStart();
						}

						myIsPlayingAnim = true;

						auto& charComp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

						charComp.currentAnimation = index;
						charComp.currentStartTime = Volt::AnimationManager::globalClock;
					}

					if (ImGui::MenuItem("Remove"))
					{
						myCurrentCharacter->RemoveAnimation(index);
						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
						break;
					}

					ImGui::EndPopup();
				}

				Volt::AssetHandle animHandle = Volt::Asset::Null();
				if (EditorUtils::AssetBrowserPopupField(popupName, animHandle, Volt::AssetType::Animation))
				{
					if (animHandle != Volt::Asset::Null())
					{
						myCurrentCharacter->SetAnimation(index, Volt::AssetManager::GetAsset<Volt::Animation>(animHandle));
					}
				}
			}

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void CharacterEditorPanel::UpdateSkeletonView()
{
	ImGui::Begin("Skeleton##charEdit");

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	//TempSkeleton skeleton{};
	//for (const auto& joint : myCurrentCharacter->GetSkeleton()->GetJoints())
	//{
	//	TempJoint tempJoint{};
	//	tempJoint.name = joint.name;

	//	if (joint.parentIndex != -1)
	//	{
	//		TempJoint* parent = nullptr;
	//		RecursiveGetParent(myCurrentCharacter->GetSkeleton()->GetNameFromJointIndex(joint.parentIndex), skeleton.joints.front(), parent);
	//		parent->childJoints.emplace_back(tempJoint);
	//	}
	//	else
	//	{
	//		skeleton.joints.emplace_back(tempJoint);
	//	}
	//}

	//for (auto& joint : skeleton.joints)
	//{
	//	RecursiveRenderJoint(joint);
	//}

	ImGui::End();
}

void CharacterEditorPanel::RecursiveGetParent(const std::string& name, TempJoint& joint, TempJoint* outJoint)
{
	if (joint.name == name)
	{
		outJoint = &joint;
		return;
	}

	for (auto& child : joint.childJoints)
	{
		RecursiveGetParent(name, child, outJoint);
	}
}

void CharacterEditorPanel::RecursiveRenderJoint(TempJoint& joint)
{
	ImGuiTreeNodeFlags flags = joint.childJoints.empty() ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : 0;
	flags |= ImGuiTreeNodeFlags_DefaultOpen;

	bool open = ImGui::TreeNodeEx(joint.name.c_str(), flags);
	if (open && !joint.childJoints.empty())
	{
		for (auto& child : joint.childJoints)
		{
			RecursiveRenderJoint(child);
		}

		ImGui::TreePop();
	}

}