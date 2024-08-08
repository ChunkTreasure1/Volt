#include "sbpch.h"
#include "CharacterEditorPanel.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Camera/EditorCameraController.h"
#include "Sandbox/Utility/Theme.h"

#include <Volt/Animation/AnimationManager.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Asset/Animation/AnimatedCharacter.h>
#include <Volt/Asset/Animation/Animation.h>
#include <Volt/Asset/Animation/Skeleton.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/Entity.h>

#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/LightComponents.h>

#include <Volt/Utility/UIUtility.h>

CharacterEditorPanel::CharacterEditorPanel()
	: EditorWindow("Character Editor", true)
{
	m_windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	myCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	myScene = Volt::Scene::CreateDefaultScene("Character Editor", false);

	myCharacterEntity = myScene->CreateEntity("Character");
	myCharacterEntity.AddComponent<Volt::AnimatedCharacterComponent>();
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
				const std::filesystem::path path = FileSystem::OpenFileDialogue({ { "Animated Character (*.vtchr)", "vtchr" }});
				if (!path.empty() && FileSystem::Exists(path))
				{
					myCurrentCharacter = Volt::AssetManager::GetAsset<Volt::AnimatedCharacter>(Volt::AssetManager::GetRelativePath(path));
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
					UI::Notify(NotificationType::Success, "Saved character!", std::format("Character {0} successfully saved!", myCurrentCharacter->assetName));
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
	UpdateAnimationTimelinePanel();
	UpdateJointAttachmentViewPanel();

	EditorUtils::NewCharacterModal("New Character##NewCharacterEditor", myCurrentCharacter, myNewCharacterData);
	AddAnimationEventModal();
	AddJointAttachmentPopup();
}

void CharacterEditorPanel::OnEvent(Volt::Event& e)
{
	if (!IsOpen()) { return; }

	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(CharacterEditorPanel::OnRenderEvent));
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(CharacterEditorPanel::OnUpdateEvent));

	myCameraController->OnEvent(e);
}

void CharacterEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	if (myCurrentCharacter)
	{
		Volt::AssetManager::Get().SaveAsset(myCurrentCharacter);
	}

	myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>().animatedCharacter = asset->handle;
	myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>().attachedEntities.clear();

	for (const auto& attachmentEntity : myJointAttachmentEntities)
	{
		myScene->RemoveEntity(attachmentEntity);
	}
	myJointAttachmentEntities.clear();

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

		for (const auto& attachment : myCurrentCharacter->GetJointAttachments())
		{
			auto newEntity = myScene->CreateEntity();
			newEntity.AddComponent<Volt::MeshComponent>().handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtasset")->handle;
			newEntity.SetScale(0.2f);

			myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>().attachedEntities[attachment.id].emplace_back(newEntity);
		}
	}
	else
	{
		mySkinHandle = Volt::Asset::Null();
		mySkeletonHandle = Volt::Asset::Null();
	}
}

void CharacterEditorPanel::OnOpen()
{
	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.scene = myScene;
		spec.debugName = "Character Editor";

		//Volt::SceneRendererSettings settings{};
		//settings.enableGrid = true;

		mySceneRenderer = CreateScope<Volt::SceneRenderer>(spec);
	}
}

void CharacterEditorPanel::OnClose()
{
	mySceneRenderer = nullptr;
}

bool CharacterEditorPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	mySceneRenderer->OnRenderEditor(myCameraController->GetCamera());
	return false;
}

bool CharacterEditorPanel::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (myCurrentCharacter && myCurrentCharacter->IsValid())
	{
		if (myIsPlayingAnim)
		{
			Volt::AnimationManager::Update(e.GetTimestep());

			auto& comp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

			if (comp.isLooping && comp.currentStartTime + myCurrentCharacter->GetAnimationDuration(comp.currentAnimation) <= Volt::AnimationManager::globalClock)
			{
				comp.currentStartTime = Volt::AnimationManager::globalClock;
			}
		}
		else if (mySelectedKeyframe != -1 && !myIsPlayingAnim)
		{
			auto& comp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

			Ref<Volt::Animation> currentAnimation = myCurrentCharacter->GetAnimations().at((uint32_t)mySelectedAnimation);

			float selectedStepTime = (myCurrentCharacter->GetAnimationDuration(comp.currentAnimation) / currentAnimation->GetFrameCount()) * mySelectedKeyframe;

			comp.currentAnimation = mySelectedAnimation;
			comp.currentStartTime = selectedStepTime;
			comp.isPlaying = true;
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

	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("##toolbarCharEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ForceWindowDocked(ImGui::GetCurrentWindow());

	if (UI::ImageButton("##Save", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
	{
		if (myCurrentCharacter)
		{
			Volt::AssetManager::Get().SaveAsset(myCurrentCharacter);
			UI::Notify(NotificationType::Success, "Saved Character!", std::format("Saved character {0} to file!", myCurrentCharacter->assetName));
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##Load", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Open)), { myButtonSize, myButtonSize }))
	{
		const std::filesystem::path characterPath = FileSystem::OpenFileDialogue({ { "Animated Character (*.vtchr)", "vtchr" } });
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

	ImGui::SetNextWindowDockID(m_mainDockID, ImGuiCond_Always);
	ImGui::SetNextWindowClass(GetWindowClass());

	ImGui::Begin("Viewport##charEdit");

	myCameraController->SetIsControllable(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows));

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();

	myPerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	myPerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	ImVec2 viewportSize = ImGui::GetContentRegionAvail();
	if (myViewportSize != (*(glm::vec2*)&viewportSize) && viewportSize.x > 0 && viewportSize.y > 0)
	{
		myViewportSize = { viewportSize.x, viewportSize.y };
		mySceneRenderer->Resize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myScene->SetRenderSize((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
		myCameraController->UpdateProjection((uint32_t)myViewportSize.x, (uint32_t)myViewportSize.y);
	}

	ImGui::Image(UI::GetTextureID(mySceneRenderer->GetFinalImage()), viewportSize);
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void CharacterEditorPanel::UpdateProperties()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Properties##charEdit");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	UI::Header("Character Properties");
	ImGui::Separator();

	if (!myCharacterEntity.HasComponent<Volt::AnimatedCharacterComponent>())
	{
		ImGui::End();
		return;
	}

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
			Ref<Volt::Skeleton> skeleton = Volt::AssetManager::GetAsset<Volt::Skeleton>(mySkeletonHandle);
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

	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Animations##charEdit");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	UI::Header("Animations");
	ImGui::Separator();

	ImGui::BeginChild("AnimBar", { ImGui::GetContentRegionAvail().x, myButtonSize + 5.f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.f, 0.f });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

		// Adding
		{
			Volt::AssetHandle addHandle = Volt::Asset::Null();
			if (UI::ImageButton("##Add", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Add)), { buttonSize, buttonSize }))
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
			Ref<Volt::Texture2D> icon = myIsPlayingAnim ? EditorResources::GetEditorIcon(EditorIcon::Stop) : EditorResources::GetEditorIcon(EditorIcon::Play);
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
			ImGui::TableSetupColumn("Animation", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (const auto& [index, anim] : myCurrentCharacter->GetAnimations())
			{
				ImGui::TableNextColumn();

				std::string animName;
				if (anim && anim->IsValid())
				{
					animName = anim->assetName;
				}
				else
				{
					"Null";
				}

				std::string popupName = "animPopup" + std::to_string(index);

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				UI::InputText("", animName, ImGuiInputTextFlags_ReadOnly);
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				{
					ImGui::OpenPopup(popupName.c_str());
				}
				else if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					mySelectedAnimation = (int32_t)index;
					mySelectedKeyframe = -1;
				}
				ImGui::PopItemWidth();

				Volt::AssetHandle animHandle = Volt::Asset::Null();
				if (EditorUtils::AssetBrowserPopupField(popupName, animHandle, Volt::AssetType::Animation))
				{
					if (animHandle != Volt::Asset::Null())
					{
						myCurrentCharacter->SetAnimation(index, Volt::AssetManager::GetAsset<Volt::Animation>(animHandle));
					}
				}

				ImGui::TableNextColumn();

				// Play
				{
					auto& charComp = myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>();

					Ref<Volt::Texture2D> icon = (myIsPlayingAnim && charComp.currentAnimation == index) ? EditorResources::GetEditorIcon(EditorIcon::Stop) : EditorResources::GetEditorIcon(EditorIcon::Play);
					const std::string playId = "##Play" + std::to_string(index);

					if (UI::ImageButton(playId, UI::GetTextureID(icon), { 20.f, 20.f }))
					{
						if (myIsPlayingAnim && index != charComp.currentAnimation)
						{
							charComp.currentAnimation = index;
							charComp.currentStartTime = Volt::AnimationManager::globalClock;
						}
						else
						{
							myIsPlayingAnim = !myIsPlayingAnim;
							if (myIsPlayingAnim)
							{
								myScene->OnRuntimeStart();

								charComp.currentAnimation = index;
								charComp.currentStartTime = Volt::AnimationManager::globalClock;
								charComp.isPlaying = true;
							}
							else
							{
								charComp.isPlaying = false;

								myScene->OnRuntimeEnd();
							}
						}
					}
				}

				ImGui::SameLine();

				// Remove
				{
					auto id = UI::GetID();

					std::string strId = "-##" + std::to_string(id);

					if (ImGui::Button(strId.c_str(), { 20.f, 20.f }))
					{
						myCurrentCharacter->RemoveAnimation(index);
						break;
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
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Skeleton##charEdit");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	UI::Header("Skeleton");
	ImGui::Separator();

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	if (!myCurrentCharacter->GetSkin() || !myCurrentCharacter->GetSkeleton())
	{
		ImGui::End();
		return;
	}

	TempSkeleton skeleton{};
	for (const auto& joint : myCurrentCharacter->GetSkeleton()->GetJoints())
	{
		TempJoint tempJoint{};
		tempJoint.name = joint.name;

		if (joint.parentIndex != -1)
		{
			TempJoint* parent = nullptr;
			RecursiveGetParent(myCurrentCharacter->GetSkeleton()->GetNameFromJointIndex(joint.parentIndex), skeleton.joints.front(), parent);
			if (parent)
			{
				parent->childJoints.emplace_back(tempJoint);
			}
		}
		else
		{
			skeleton.joints.emplace_back(tempJoint);
		}
	}

	RecursiveRenderJoint(skeleton.joints.at(0));

	ImGui::End();
}

void CharacterEditorPanel::UpdateAnimationTimelinePanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Animation Timeline##charEdit", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	ForceWindowDocked(ImGui::GetCurrentWindow());

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	if (myCurrentCharacter->GetAnimationCount() == 0 || mySelectedAnimation == -1 || !myCurrentCharacter->GetAnimations().contains((uint32_t)mySelectedAnimation))
	{
		ImGui::End();
		return;
	}

	Ref<Volt::Animation> currentAnimation = myCurrentCharacter->GetAnimations().at((uint32_t)mySelectedAnimation);

	if (!currentAnimation)
	{
		ImGui::End();
		return;
	}

	const auto duration = currentAnimation->GetDuration();
	const int32_t stepCount = (int32_t)currentAnimation->GetFrameCount();

	if (ImGui::BeginTable("timelineTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextColumn();
		if (ImGui::BeginChild("eventsChild", { ImGui::GetColumnWidth(), ImGui::GetContentRegionAvail().y }, true))
		{
			UI::Header("Created Events");
			ImGui::Separator();
			if (myCurrentCharacter->HasAnimationEvents((uint32_t)mySelectedAnimation))
			{
				const auto& events = myCurrentCharacter->GetAnimationEvents((uint32_t)mySelectedAnimation);
				for (int index = 0; index < events.size(); index++)
				{
					const auto id = UI::GetID();
					bool selected = false;

					ImGui::Selectable(std::format("{0}: ", events[index].name).c_str(), &selected, ImGuiSelectableFlags_AllowItemOverlap, ImVec2(150, 25));
					ImGui::SameLine();
					ImGui::DragInt(std::format("-##rem{0}", id).c_str(), (int*)&events[index].frame);

					std::string popupName = "eventPopup" + std::to_string(index);
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup(popupName.c_str());
					}

					std::string rightClickId = "eventRightClick" + std::to_string(index);
					if (ImGui::BeginPopupContextItem(rightClickId.c_str(), ImGuiPopupFlags_MouseButtonRight))
					{
						if (ImGui::MenuItem("Remove"))
						{
							myCurrentCharacter->RemoveAnimationEvent(events[index].name, events[index].frame, (uint32_t)mySelectedAnimation);
							ImGui::CloseCurrentPopup();
							ImGui::EndPopup();
							break;
						}
						ImGui::EndPopup();
					}

				}
			}

			ImGui::EndChild();
		}

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("keyframeChild", ImGui::GetContentRegionAvail()))
		{
			if (ImGui::BeginChild("timeChild", { ImGui::GetContentRegionAvail().x, 35.f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImGui::TextUnformatted("0.00");
				ImGui::SameLine();

				auto drawList = ImGui::GetWindowDrawList();
				ImVec2 windowPos = ImGui::GetWindowPos();
				ImVec2 windowSize = ImGui::GetWindowSize();
				ImGuiIO& io = ImGui::GetIO();

				drawList->AddRectFilled(windowPos, windowPos + windowSize, IM_COL32(50, 50, 50, 255));

				UI::ScopedColor color{ ImGuiCol_Button, { 1.f, 1.f, 1.f, 1.f } };
				{
					constexpr float padding = 3.f;
					UI::ScopedStyleFloat2 itemPadding{ ImGuiStyleVar_ItemSpacing, { 3.f, 0.f } };

					const float stepSize = windowSize.x / stepCount;
					for (int32_t i = 0; i < stepCount; i++)
					{
						//ImGui::Button(("K##" + std::to_string(i)).c_str(), { stepSize, 20.f });
						ImVec2 keyMinPos = ImVec2(windowPos.x + (stepSize * i) + padding, windowPos.y);
						ImVec2 keyMaxPos = ImVec2(windowPos.x + (stepSize * (i + 1)) - padding, windowPos.y + windowSize.y);

						if (mySelectedKeyframe == i)
						{
							drawList->AddRectFilled(keyMinPos, keyMaxPos, IM_COL32(100, 100, 100, 255));
						}
						else
						{
							drawList->AddRectFilled(keyMinPos, keyMaxPos, IM_COL32(255, 255, 255, 255));
						}

						ImGui::ItemAdd(ImRect(keyMinPos, keyMaxPos), i);

						if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
						{
							mySelectedKeyframe = i;
						}

						if (ImGui::IsItemHovered())
						{
							drawList->AddText(io.MousePos + ImVec2(0, -20), IM_COL32(255, 100, 100, 255), std::to_string(i).c_str());
						}

						if (ImGui::BeginPopupContextItem(("item##" + std::to_string(i)).c_str(), ImGuiPopupFlags_MouseButtonRight))
						{
							if (ImGui::MenuItem("Add Event"))
							{
								UI::OpenModal("Add Animation Event");
								myAddAnimEventData = {};
								myAddAnimEventData.frame = i;
							}

							ImGui::EndPopup();
						}

						ImGui::SameLine();
					}

					VT_LOG(Trace, "x: {0}, y: {1}", windowPos.x, windowPos.y);
				}

				ImGui::Text("%.2f", duration);

				ImGui::EndChild();
			}

			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void CharacterEditorPanel::UpdateJointAttachmentViewPanel()
{
	ImGui::SetNextWindowClass(GetWindowClass());
	ImGui::Begin("Joint Attachments##charEdit");
	ForceWindowDocked(ImGui::GetCurrentWindow());

	UI::Header("Joint Attachments");
	ImGui::Separator();

	if (!myCurrentCharacter)
	{
		ImGui::End();
		return;
	}

	if (!myCurrentCharacter->GetSkin() || !myCurrentCharacter->GetSkeleton())
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Add"))
	{
		UI::OpenPopup("addJointAttachment");
		myJointSearchQuery = "";
		myActivateJointSearch = true;
	}

	auto& jointAttachments = const_cast<Vector<Volt::AnimatedCharacter::JointAttachment>&>(myCurrentCharacter->GetJointAttachments());

	const auto totalWidth = ImGui::GetContentRegionAvail().x;

	if (ImGui::BeginTable("AttachmentTable", 3, ImGuiTableFlags_BordersInnerH, ImGui::GetContentRegionAvail()))
	{
		ImGui::TableSetupColumn("Joint");
		ImGui::TableSetupColumn("Attachment Name");

		ImGui::TableHeadersRow();

		int32_t indexToRemove = -1;
		for (int32_t index = 0; auto& attachment : jointAttachments)
		{
			ImGui::TableNextColumn();

			auto jointName = myCurrentCharacter->GetSkeleton()->GetNameFromJointIndex(attachment.jointIndex);

			ImGui::PushItemWidth(totalWidth - 11.f);
			const std::string jntId = "##" + std::to_string(UI::GetID());

			ImGui::InputTextString(jntId.c_str(), &jointName, ImGuiInputTextFlags_ReadOnly);
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			ImGui::PushItemWidth(totalWidth - 11.f);
			
			const std::string attId = "##" + std::to_string(UI::GetID());
			ImGui::InputTextString(attId.c_str(), &attachment.name);

			std::string popupName = "offsetRightclick" + std::to_string(index);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup(popupName.c_str());
			}

			std::string rightClickId = "offsetRightclick" + std::to_string(index);
			if (ImGui::BeginPopupContextItem(rightClickId.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				UI::BeginProperties("OFFSET");
				ImGui::Text("OFFSET");

				UI::Property("Pos", attachment.positionOffset);
				UI::Property("Rot", attachment.rotationOffset);

				UI::EndProperties();
				ImGui::EndPopup();
			}

			ImGui::PopItemWidth();
		
			ImGui::TableNextColumn();
			if (ImGui::Button((std::string("-##") + std::to_string(index)).c_str(), { 22.f, 22.f }))
			{
				indexToRemove = index;
			}

			index++;
		}

		if (indexToRemove > -1)
		{
			jointAttachments.erase(jointAttachments.begin() + indexToRemove);
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void CharacterEditorPanel::AddAnimationEventModal()
{
	if (UI::BeginModal("Add Animation Event", ImGuiWindowFlags_AlwaysAutoResize))
	{
		UI::PushID();
		if (UI::BeginProperties("animEvent"))
		{
			UI::Property("Name", myAddAnimEventData.name);
			UI::EndProperties();
		}
		UI::PopID();

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			myCurrentCharacter->AddAnimationEvent(myAddAnimEventData.name, myAddAnimEventData.frame, (uint32_t)mySelectedAnimation);
			ImGui::CloseCurrentPopup();
		}

		UI::EndModal();
	}
}

void CharacterEditorPanel::AddJointAttachmentPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });

	if (UI::BeginPopup("addJointAttachment", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		Vector<std::string> jointNames;
		for (const auto& joint : myCurrentCharacter->GetSkeleton()->GetJoints())
		{
			jointNames.emplace_back(joint.name);
		}

		// Search bar
		{
			bool t;
			EditorUtils::SearchBar(myJointSearchQuery, t, myActivateJointSearch);
			if (myActivateJointSearch)
			{
				myActivateJointSearch = false;
			}
		}

		if (!myJointSearchQuery.empty())
		{
			jointNames = UI::GetEntriesMatchingQuery(myJointSearchQuery, jointNames);
		}

		// List child
		{
			auto& jointAttachments = const_cast<Vector<Volt::AnimatedCharacter::JointAttachment>&>(myCurrentCharacter->GetJointAttachments());

			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			for (const auto& name : jointNames)
			{
				const std::string id = name + "##" + std::to_string(UI::GetID());

				UI::ShiftCursor(4.f, 0.f);
				UI::RenderMatchingTextBackground(myJointSearchQuery, name, EditorTheme::MatchingTextBackground);
				if (ImGui::MenuItem(id.c_str()))
				{
					auto& newAttachment = jointAttachments.emplace_back();
					newAttachment.name = "New Attachment";
					newAttachment.jointIndex = myCurrentCharacter->GetSkeleton()->GetJointIndexFromName(name);

					auto newEntity = myScene->CreateEntity();
					newEntity.AddComponent<Volt::MeshComponent>().handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtasset")->handle;
					newEntity.SetScale(0.2f);

					myCharacterEntity.GetComponent<Volt::AnimatedCharacterComponent>().attachedEntities[newAttachment.id].emplace_back(newEntity);

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndChild();
		}

		UI::EndPopup();
	}
}

void CharacterEditorPanel::RecursiveGetParent(const std::string& name, TempJoint& joint, TempJoint*& outJoint)
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
