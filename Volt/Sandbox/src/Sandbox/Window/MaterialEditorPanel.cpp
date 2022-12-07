#include "sbpch.h"
#include "MaterialEditorPanel.h"

#include "Sandbox/Window/EditorIconLibrary.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"

#include <Volt/Asset/Mesh/MaterialRegistry.h>
#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/Framebuffer.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

MaterialEditorPanel::MaterialEditorPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Material Editor", true), myEditorScene(aScene)
{
	myPreviewCamera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 300.f);
	myPreviewCamera->SetPosition({ 0.f, 0.f, -200.f });

	myPreviewScene = CreateRef<Volt::Scene>();

	// Material sphere
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::MeshComponent& comp = entity.AddComponent<Volt::MeshComponent>();
		comp.handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Sphere.vtmesh")->handle;
		myPreviewEntity = entity;
	}

	// Skylight
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::SkylightComponent& comp = entity.AddComponent<Volt::SkylightComponent>();
		comp.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Assets/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
	}

	// Directional light
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::DirectionalLightComponent& comp = entity.AddComponent<Volt::DirectionalLightComponent>();
		comp.castShadows = false;
		comp.intensity = 2.f;

		entity.SetLocalRotation(gem::quat(gem::radians(gem::vec3{ 70.f, 0.f, 100.f })));
	}

	myPreviewRenderer = CreateRef<Volt::SceneRenderer>(myPreviewScene);
	myPreviewRenderer->Resize(1024, 1024);
}

void MaterialEditorPanel::UpdateMainContent()
{}

void MaterialEditorPanel::UpdateContent()
{
	UpdateMaterials();
	UpdateSubMaterials();
	UpdatePreview();
	UpdateProperties();
	UpdateToolbar();
}

void MaterialEditorPanel::OpenAsset(Ref<Volt::Asset> asset)
{
	mySelectedMaterial = std::reinterpret_pointer_cast<Volt::Material>(asset);
	mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);

	myPreviewEntity.GetComponent<Volt::MeshComponent>().overrideMaterial = mySelectedMaterial->handle;
}

void MaterialEditorPanel::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(MaterialEditorPanel::OnRenderEvent));
}

bool MaterialEditorPanel::OnRenderEvent(Volt::AppRenderEvent& e)
{
	myPreviewRenderer->OnRenderEditor(myPreviewCamera);

	return false;
}

void MaterialEditorPanel::UpdateToolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
	UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
	UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
	UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

	ImGui::Begin("##toolbarMatEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (UI::ImageButton("##Save", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
	{
		if (mySelectedMaterial)
		{
			if (FileSystem::IsWriteable(mySelectedMaterial->path))
			{
				Volt::AssetManager::Get().SaveAsset(mySelectedMaterial);
				UI::Notify(NotificationType::Success, "Material saved!", std::format("Material {0} was saved!", mySelectedMaterial->path.string()));
			}
			else
			{
				UI::Notify(NotificationType::Error, "Unable to save material!", "Unable to save material, it is not writeable!");
			}

		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##Reload", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::Reload)), { myButtonSize, myButtonSize }))
	{
		if (mySelectedMaterial)
		{
			Volt::AssetManager::Get().ReloadAsset(mySelectedMaterial->handle);
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##GetMaterial", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::GetMaterial)), { myButtonSize, myButtonSize }))
	{
		if (SelectionManager::GetSelectedCount() > 0)
		{
			auto entity = SelectionManager::GetSelectedEntities().front();
			auto& registry = myEditorScene->GetRegistry();

			if (registry.HasComponent<Volt::MeshComponent>(entity))
			{
				auto& meshComp = registry.GetComponent<Volt::MeshComponent>(entity);
				if (meshComp.overrideMaterial != Volt::Asset::Null())
				{
					mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Material>(meshComp.overrideMaterial);
					mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);
				}
				else
				{
					if (meshComp.handle != Volt::Asset::Null())
					{
						mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle)->GetMaterial();
						mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);
					}
				}

				myPreviewEntity.GetComponent<Volt::MeshComponent>().overrideMaterial = mySelectedMaterial->handle;
				myPreviewEntity.GetComponent<Volt::MeshComponent>().subMaterialIndex = 0;
			}
		}
	}

	ImGui::SameLine();

	if (UI::ImageButton("##SetMaterial", UI::GetTextureID(EditorIconLibrary::GetIcon(EditorIcon::SetMaterial)), { myButtonSize, myButtonSize }))
	{
		if (SelectionManager::GetSelectedCount() > 0)
		{
			auto entity = SelectionManager::GetSelectedEntities().front();
			auto& registry = myEditorScene->GetRegistry();

			if (registry.HasComponent<Volt::MeshComponent>(entity))
			{
				auto& meshComp = registry.GetComponent<Volt::MeshComponent>(entity);
				meshComp.overrideMaterial = mySelectedMaterial->handle;
			}
		}
	}

	ImGui::PopStyleVar(2);
	ImGui::End();
}

void MaterialEditorPanel::UpdateProperties()
{
	ImGui::Begin("Properties##materialEditor");
	{
		if (mySelectedSubMaterial)
		{
			UI::Header("Properties");
			std::vector<std::string> shaderNames;
			for (const auto& [name, shader] : Volt::ShaderRegistry::GetAllShaders())
			{
				if (!shader->IsInternal())
				{
					shaderNames.emplace_back(name);
				}
			}

			UI::PushId();
			if (UI::BeginProperties("Shader"))
			{
				auto& materialBuffer = const_cast<Volt::Shader::MaterialBuffer&>(mySelectedSubMaterial->GetResources().materialBuffer);

				int32_t selected = 0;
				const std::string shaderName = Utils::ToLower(mySelectedSubMaterial->GetShader()->GetName());

				auto it = std::find(shaderNames.begin(), shaderNames.end(), shaderName);
				selected = (int32_t)std::distance(shaderNames.begin(), it);

				if (UI::ComboProperty("Shader", selected, shaderNames))
				{
					mySelectedSubMaterial->SetShader(Volt::ShaderRegistry::Get(shaderNames[selected]));
				}

				bool changed = false;

				for (const auto& [name, param] : materialBuffer.parameters)
				{
					switch (param.type)
					{
						case Volt::ElementType::Bool: changed = UI::Property(name, *(bool*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::Int: changed = UI::Property(name, *(int32_t*)&materialBuffer.data[param.offset]); break;

						case Volt::ElementType::UInt: changed = UI::Property(name, *(uint32_t*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::UInt2: changed = UI::Property(name, *(gem::vec2ui*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::UInt3: changed = UI::Property(name, *(gem::vec3ui*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::UInt4: changed = UI::Property(name, *(gem::vec4ui*)&materialBuffer.data[param.offset]); break;

						case Volt::ElementType::Float: changed = UI::Property(name, *(float*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::Float2: changed = UI::Property(name, *(gem::vec2*)&materialBuffer.data[param.offset]); break;
						case Volt::ElementType::Float3:
						{
							const std::string lowerName = Utils::ToLower(name);

							if (lowerName.contains("color"))
							{
								changed = UI::PropertyColor(name, *(gem::vec3*)&materialBuffer.data[param.offset]);
							}
							else
							{
								changed = UI::Property(name, *(gem::vec3*)&materialBuffer.data[param.offset]);
							}

							break;
						}
						case Volt::ElementType::Float4:
						{
							const std::string lowerName = Utils::ToLower(name);

							if (lowerName.contains("color"))
							{
								changed = UI::PropertyColor(name, *(gem::vec4*)&materialBuffer.data[param.offset]);
							}
							else
							{
								changed = UI::Property(name, *(gem::vec4*)&materialBuffer.data[param.offset]);
							}

							break;
						}
					}

					if (changed)
					{
						mySelectedSubMaterial->UpdateBuffer();
					}
				}


				UI::EndProperties();
			}
			UI::PopId();

			ImGui::Separator();

			UI::Header("Textures");

			UI::PushId();
			if (UI::BeginProperties("Textures"))
			{
				const auto& textureDefinitions = mySelectedSubMaterial->GetResources().shaderTextureDefinitions;
				const auto& textures = mySelectedSubMaterial->GetTextures();

				for (const auto& [binding, name] : textureDefinitions)
				{
					auto it = textures.find(binding);
					if (it == textures.end())
					{
						VT_CORE_CRITICAL("Texture not found in material! Something has gone terribly wrong!");
						continue;
					}

					Volt::AssetHandle textureHandle = it->second->handle;

					if (EditorUtils::Property(name, textureHandle, Volt::AssetType::Texture))
					{
						auto newTex = Volt::AssetManager::GetAsset<Volt::Texture2D>(textureHandle);

						if (newTex && newTex->IsValid())
						{
							mySelectedSubMaterial->SetTexture(binding, newTex);
						}
					}
				}

				UI::EndProperties();
			}
			UI::PopId();
		}
	}
	ImGui::End();

}

void MaterialEditorPanel::UpdatePreview()
{
	ImGui::Begin("Preview##materialEditor");
	{
		const auto size = ImGui::GetContentRegionAvail();
		myPreviewCamera->SetPerspectiveProjection(60.f, size.x / size.y, 1.f, 300.f);

		ImGui::Image(UI::GetTextureID(myPreviewRenderer->GetFinalFramebuffer()->GetColorAttachment(0)), size);
	}
	ImGui::End();
}

void MaterialEditorPanel::UpdateSubMaterials()
{
	ImGui::Begin("Sub materials");
	{
		if (mySelectedMaterial)
		{
			static float padding = 16.f;

			const float thumbnailSize = 85.f;
			float cellSize = thumbnailSize + padding;
			float panelWidth = ImGui::GetContentRegionAvail().x;
			int32_t columnCount = (int32_t)(panelWidth / cellSize);

			if (columnCount < 1)
			{
				columnCount = 1;
			}

			ImGui::Columns(columnCount, nullptr, false);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.1f, 0.1f, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 1.f, 0.f, 0.f, 1.f });

			const auto& subMaterials = mySelectedMaterial->GetSubMaterials();
			for (auto& [index, material] : subMaterials)
			{
				ImGui::PushID((uint32_t)material->GetName().c_str());

				const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize();
				const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();
				const ImVec2 minChild = AssetBrowserUtilities::GetBrowserItemMinPos();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.f, 0.f, 0.f, 0.f });
				ImGui::BeginChild("hoverWindow", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				ImGui::PopStyleColor();

				{
					const bool itemHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

					ImVec4 childBgCol = AssetBrowserUtilities::GetBrowserItemDefaultColor();

					if (itemHovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
					{
						childBgCol = AssetBrowserUtilities::GetBrowserItemClickedColor();
					}
					else if (itemHovered)
					{
						childBgCol = AssetBrowserUtilities::GetBrowserItemHoveredColor();
					}
					else if (material == mySelectedSubMaterial)
					{
						childBgCol = AssetBrowserUtilities::GetBrowserItemSelectedColor();
					}

					ImGui::PushStyleColor(ImGuiCol_ChildBg, childBgCol);
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.f);
					ImGui::BeginChild("item", itemSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
					{
						UI::ShiftCursor(itemPadding / 2.f, 10.f);

						ImGui::BeginChild("image", { thumbnailSize, thumbnailSize }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
						{
							ImGui::Image(UI::GetTextureID(Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/AssetIcons/icon_material.dds")), { thumbnailSize, thumbnailSize });
						}
						ImGui::EndChild();
						ImGui::PopStyleVar();

						UI::ShiftCursor(itemPadding / 2.f, 0.f);

						ImGui::TextWrapped(material->GetName().c_str());

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && itemHovered)
						{
							mySelectedSubMaterial = material;
							myPreviewEntity.GetComponent<Volt::MeshComponent>().subMaterialIndex = index;
						}
					}

					ImGui::EndChild();
				}

				ImGui::EndChild();

				ImGui::PopStyleColor();
				ImGui::NextColumn();

				ImGui::PopID();
			}

			ImGui::PopStyleColor(2);
		}
	}
	ImGui::End();
}

void MaterialEditorPanel::UpdateMaterials()
{
	ImGui::Begin("Materials", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
	{
		const auto& materials = Volt::MaterialRegistry::GetMaterials();

		if (EditorUtils::SearchBar(mySearchQuery, myHasSearchQuery))
		{
		}

		ImGui::BeginChild("Scrollable");
		{
			for (auto& [name, material] : materials)
			{
				if (myHasSearchQuery && !Utils::ToLower(name).contains(Utils::ToLower(mySearchQuery)))
				{
					continue;
				}

				bool selected = false;
				if (mySelectedMaterial)
				{
					selected = material == mySelectedMaterial->path;
				}

				if (ImGui::Selectable(name.c_str(), &selected))
				{
					mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Material>(material);
					mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);

					myPreviewEntity.GetComponent<Volt::MeshComponent>().overrideMaterial = mySelectedMaterial->handle;
					myPreviewEntity.GetComponent<Volt::MeshComponent>().subMaterialIndex = 0;
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}