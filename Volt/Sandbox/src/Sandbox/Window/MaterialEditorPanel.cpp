#include "sbpch.h"
#include "MaterialEditorPanel.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Sandbox/Utility/AssetBrowserUtilities.h"

#include <Volt/Asset/Mesh/SubMaterial.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/Texture/Texture2D.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/RenderingNew/SceneRendererNew.h>
#include <Volt/Rendering/VulkanFramebuffer.h>
#include <Volt/Rendering/Renderer.h>
#include <Volt/Rendering/RenderPipeline/RenderPipeline.h>
#include <Volt/Rendering/RenderPipeline/ShaderRegistry.h>

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/LightComponents.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>

MaterialEditorPanel::MaterialEditorPanel(Ref<Volt::Scene>& aScene)
	: EditorWindow("Material Editor", true), myEditorScene(aScene)
{
	myPreviewCamera = CreateRef<Volt::Camera>(60.f, 1.f, 1.f, 300.f);
	myPreviewCamera->SetPosition({ 0.f, 0.f, -200.f });

	myPreviewScene = Volt::Scene::CreateDefaultScene("Material Editor", false);

	// Material sphere
	{
		auto entity = myPreviewScene->CreateEntity();
		Volt::MeshComponent& comp = entity.AddComponent<Volt::MeshComponent>();
		comp.handle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Meshes/Primitives/SM_Sphere.vtmesh");
		myPreviewEntity = entity;
	}

	// Set HDRI
	{
		auto skylightEntities = myPreviewScene->GetAllEntitiesWith<Volt::SkylightComponent>();

		Volt::Entity ent{ skylightEntities.front(), myPreviewScene.get() };
		ent.GetComponent<Volt::SkylightComponent>().environmentHandle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Textures/HDRIs/defaultHDRI.hdr");
	}
}

void MaterialEditorPanel::UpdateMainContent()
{
}

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

	myPreviewEntity.GetComponent<Volt::MeshComponent>().material = mySelectedMaterial->handle;
}

void MaterialEditorPanel::OnEvent(Volt::Event& e)
{
	if (!IsOpen()) { return; }

	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(MaterialEditorPanel::OnRenderEvent));
}

void MaterialEditorPanel::OnOpen()
{
	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Mesh Preview";
		spec.scene = myPreviewScene;
		spec.initialResolution = { 1024 };

		myPreviewRenderer = CreateRef<Volt::SceneRendererNew>(spec);
	}

	// Set HDRI
	{
		auto skylightEntities = myPreviewScene->GetAllEntitiesWith<Volt::SkylightComponent>();

		Volt::Entity ent{ skylightEntities.front(), myPreviewScene.get() };
		ent.GetComponent<Volt::SkylightComponent>().environmentHandle = Volt::AssetManager::GetAssetHandleFromFilePath("Engine/Textures/HDRIs/defaultHDRI.hdr");
	}
}

void MaterialEditorPanel::OnClose()
{
	myPreviewRenderer = nullptr;
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

	ImGui::SetNextWindowClass(GetWindowClass());

	if (ImGui::Begin("##toolbarMatEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		if (UI::ImageButton("##Save", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Save)), { myButtonSize, myButtonSize }))
		{
			if (mySelectedMaterial)
			{
				if (FileSystem::IsWriteable(Volt::AssetManager::GetFilesystemPath(mySelectedMaterial->handle)))
				{
					Volt::AssetManager::Get().SaveAsset(mySelectedMaterial);
					UI::Notify(NotificationType::Success, "Material saved!", std::format("Material {0} was saved!", mySelectedMaterial->assetName));
				}
				else
				{
					UI::Notify(NotificationType::Error, "Unable to save material!", "Unable to save material, it is not writeable!");
				}

			}
		}

		ImGui::SameLine();

		if (UI::ImageButton("##Reload", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::Reload)), { myButtonSize, myButtonSize }))
		{
			if (mySelectedMaterial)
			{
				Volt::AssetManager::Get().ReloadAsset(mySelectedMaterial->handle);
			}
		}

		ImGui::SameLine();

		if (UI::ImageButton("##GetMaterial", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::GetMaterial)), { myButtonSize, myButtonSize }))
		{
			if (SelectionManager::GetSelectedCount() > 0)
			{
				auto id = SelectionManager::GetSelectedEntities().front();
				Volt::Entity entity{ id, myEditorScene };

				if (entity.HasComponent<Volt::MeshComponent>())
				{
					auto& meshComp = entity.GetComponent<Volt::MeshComponent>();
					if (meshComp.material != Volt::Asset::Null())
					{
						mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Material>(meshComp.material);
						mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);
					}
					else
					{
						if (meshComp.GetHandle() != Volt::Asset::Null())
						{
							mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.GetHandle())->GetMaterial();
							mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);
						}
					}

					if (mySelectedMaterial)
					{
						myPreviewEntity.GetComponent<Volt::MeshComponent>().material = mySelectedMaterial->handle;
						myPreviewEntity.GetComponent<Volt::MeshComponent>().subMaterialIndex = 0;
					}
				}
			}
		}

		ImGui::SameLine();

		if (UI::ImageButton("##SetMaterial", UI::GetTextureID(EditorResources::GetEditorIcon(EditorIcon::SetMaterial)), { myButtonSize, myButtonSize }))
		{
			if (SelectionManager::GetSelectedCount() > 0 && mySelectedMaterial)
			{
				auto id = SelectionManager::GetSelectedEntities().front();
				Volt::Entity entity{ id, myEditorScene };

				if (entity.HasComponent<Volt::MeshComponent>())
				{
					auto& meshComp = entity.GetComponent<Volt::MeshComponent>();
					meshComp.material = mySelectedMaterial->handle;
				}
			}
		}
		ImGui::PopStyleVar(2);
	}

	ImGui::End();
}

void MaterialEditorPanel::UpdateProperties()
{
	ImGui::SetNextWindowClass(GetWindowClass());

	if (ImGui::Begin("Properties##materialEditor", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		if (mySelectedSubMaterial)
		{
			UI::Header("Shader");
			std::vector<std::string> shaderNames;
			shaderNames.emplace_back("None");
			for (const auto& [name, shader] : Volt::ShaderRegistry::GetShaderRegistry())
			{
				if (!shader->IsInternal())
				{
					shaderNames.emplace_back(shader->GetName());
				}
			}

			int32_t selectedShader = 0;
			const std::string shaderName = mySelectedSubMaterial->GetPipeline()->GetSpecification().shader->GetName();

			if (auto namesIt = std::find(shaderNames.begin(), shaderNames.end(), shaderName); namesIt != shaderNames.end())
			{
				selectedShader = (int32_t)std::distance(shaderNames.begin(), namesIt);
			}

			bool changed = false;

			UI::PushID();
			if (UI::BeginProperties("Shader"))
			{
				if (UI::ComboProperty("Shader", selectedShader, shaderNames))
				{
					auto newPipeline = Volt::ShaderRegistry::GetShader(shaderNames.at(selectedShader));
					if (newPipeline)
					{
						mySelectedSubMaterial->SetShader(newPipeline);

						if (shaderNames.at(selectedShader) == "Illum")
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, true);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Opaque, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Transparent, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::SSS, false);
							changed = true;
						}
						else if (shaderNames.at(selectedShader) == "IllumTransparent")
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Opaque, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Transparent, true);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::SSS, false);
							changed = true;
						}
						else if (shaderNames.at(selectedShader) == "IllumSSS")
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Opaque, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Transparent, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::SSS, true);
							changed = true;
						}
						else if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Deferred))
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, false);
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Opaque, true);
							changed = true;
						}
					}
				}

				const std::vector<const char*> materialTypes =
				{
					"Opaque",
					"Transparent",
					"Subsurface Scattering",
					"Decal"
				};

				int32_t selected = 0;
				Volt::MaterialFlag currentType = Volt::MaterialFlag::All;

				if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Opaque) || mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Deferred))
				{
					selected = 0;
					currentType = Volt::MaterialFlag::Opaque;
				}
				else if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Transparent))
				{
					selected = 1;
					currentType = Volt::MaterialFlag::Transparent;
				}
				else if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::SSS))
				{
					selected = 2;
					currentType = Volt::MaterialFlag::SSS;
				}
				else if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Decal))
				{
					selected = 3;
					currentType = Volt::MaterialFlag::Decal;
				}

				if (UI::ComboProperty("Material Type", selected, materialTypes))
				{
					if (mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::Deferred) && currentType == Volt::MaterialFlag::Opaque)
					{
						mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, false);
					}
					else
					{
						mySelectedSubMaterial->SetFlag(currentType, false);
					}

					if (selected == 0)
					{
						if (mySelectedSubMaterial->GetPipeline()->GetSpecification().shader->GetName() == "Illum")
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Deferred, true);
						}
						else
						{
							mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Opaque, true);
						}

					}
					else if (selected == 1) mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Transparent, true);
					else if (selected == 2) mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::SSS, true);
					else if (selected == 3) mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::Decal, true);

					mySelectedSubMaterial->RecompilePermutation();
					changed = true;
				}

				bool castingShadows = mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::CastShadows);
				if (UI::Property("Cast Shadows", castingShadows))
				{
					mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::CastShadows, castingShadows);
					changed = true;
				}

				bool castingAO = mySelectedSubMaterial->HasFlag(Volt::MaterialFlag::CastAO);
				if (UI::Property("Cast AO", castingAO))
				{
					mySelectedSubMaterial->SetFlag(Volt::MaterialFlag::CastAO, castingAO);
					changed = true;
				}

				UI::EndProperties();
			}
			UI::PopID();

			UI::Header("Properties");
			auto& materialData = mySelectedSubMaterial->GetMaterialData();

			UI::PushID();
			if (UI::BeginProperties("Material Data"))
			{
				changed |= UI::PropertyColor("Color", materialData.color);
				changed |= UI::PropertyColor("Emissive Color", materialData.emissiveColor);
				changed |= UI::Property("Emissive Strength", materialData.emissiveStrength, 0.f, 20.f);
				changed |= UI::Property("Roughness", materialData.roughness, 0.f, 1.f);
				changed |= UI::Property("Metalness", materialData.metalness, 0.f, 1.f);
				changed |= UI::Property("Normal Strength", materialData.normalStrength, -1.f, 1.f);

				UI::EndProperties();
			}
			UI::PopID();

			if (changed)
			{
				Volt::Renderer::UpdateMaterial(mySelectedSubMaterial.get());
			}

			if (ImGui::CollapsingHeader("Advanced"))
			{
				if (UI::BeginProperties("Advanced"))
				{
					const std::vector<const char*> topologyStrings = { "Triangle List", "Line List", "Triangle Strip", "Path List", "Point List" };
					auto currentTopology = mySelectedSubMaterial->GetTopology();

					if (UI::ComboProperty("Topology", *(int32_t*)&currentTopology, topologyStrings))
					{
						mySelectedSubMaterial->SetTopology(currentTopology);
					}

					const std::vector<const char*> cullModeStrings = { "Front", "Back", "Front And Back", "None" };
					auto currentCullMode = mySelectedSubMaterial->GetCullMode();

					if (UI::ComboProperty("Cull Mode", *(int32_t*)&currentCullMode, cullModeStrings))
					{
						mySelectedSubMaterial->SetCullMode(currentCullMode);
					}

					const std::vector<const char*> fillModeStrings = { "Solid", "Wireframe" };
					auto currentFillMode = mySelectedSubMaterial->GetFillMode();

					if (UI::ComboProperty("Fill Mode", *(int32_t*)&currentFillMode, fillModeStrings))
					{
						mySelectedSubMaterial->SetFillMode(currentFillMode);
					}

					const std::vector<const char*> depthModeStrings = { "Read", "Write", "Read/Write", "None" };
					auto currentDepthMode = mySelectedSubMaterial->GetDepthMode();

					if (UI::ComboProperty("Depth Mode", *(int32_t*)&currentDepthMode, depthModeStrings))
					{
						mySelectedSubMaterial->SetDepthMode(currentDepthMode);
					}

					UI::EndProperties();
				}

			}

			ImGui::Separator();

			UI::Header("Textures");
			const auto& textureDefinitions = mySelectedSubMaterial->GetPipeline()->GetSpecification().shader->GetResources().shaderTextureDefinitions;

			UI::PushID();
			if (UI::BeginProperties("Textures"))
			{
				const auto& textures = mySelectedSubMaterial->GetTextures();

				// Default textures
				{
					if (textures.contains("albedo"))
					{
						Volt::AssetHandle textureHandle = Volt::Asset::Null();
						if (textures.at("albedo"))
						{
							textureHandle = textures.at("albedo")->handle;
						}

						if (EditorUtils::Property("Albedo", textureHandle, Volt::AssetType::Texture))
						{
							auto newTex = Volt::AssetManager::GetAsset<Volt::Texture2D>(textureHandle);

							if (newTex && newTex->IsValid())
							{
								mySelectedSubMaterial->SetTexture("albedo", newTex);
							}
							else
							{
								mySelectedSubMaterial->SetTexture("albedo", Volt::Renderer::GetDefaultData().whiteTexture);
							}
						}
					}

					if (textures.contains("normal"))
					{
						Volt::AssetHandle textureHandle = Volt::Asset::Null();
						if (textures.at("normal"))
						{
							textureHandle = textures.at("normal")->handle;
						}

						if (EditorUtils::Property("Normal", textureHandle, Volt::AssetType::Texture))
						{
							auto newTex = Volt::AssetManager::GetAsset<Volt::Texture2D>(textureHandle);

							if (newTex && newTex->IsValid())
							{
								mySelectedSubMaterial->SetTexture("normal", newTex);
							}
							else
							{
								mySelectedSubMaterial->SetTexture("normal", Volt::Renderer::GetDefaultData().whiteTexture);
							}
						}
					}

					if (textures.contains("material"))
					{
						Volt::AssetHandle textureHandle = Volt::Asset::Null();
						if (textures.at("material"))
						{
							textureHandle = textures.at("material")->handle;
						}

						if (EditorUtils::Property("Material", textureHandle, Volt::AssetType::Texture))
						{
							auto newTex = Volt::AssetManager::GetAsset<Volt::Texture2D>(textureHandle);

							if (newTex && newTex->IsValid())
							{
								mySelectedSubMaterial->SetTexture("material", newTex);
							}
							else
							{
								mySelectedSubMaterial->SetTexture("material", Volt::Renderer::GetDefaultData().whiteTexture);
							}
						}
					}
				}

				for (const auto& [textureShaderName, editorName] : textureDefinitions)
				{
					if (!textures.contains(textureShaderName))
					{
						continue;
					}

					Volt::AssetHandle textureHandle = Volt::Asset::Null();
					if (textures.at(textureShaderName))
					{
						textureHandle = textures.at(textureShaderName)->handle;
					}

					if (EditorUtils::Property(editorName, textureHandle, Volt::AssetType::Texture))
					{
						auto newTex = Volt::AssetManager::GetAsset<Volt::Texture2D>(textureHandle);

						if (newTex && newTex->IsValid())
						{
							mySelectedSubMaterial->SetTexture(textureShaderName, newTex);
						}
						else
						{
							mySelectedSubMaterial->SetTexture(textureShaderName, Volt::Renderer::GetDefaultData().whiteTexture);
						}
					}
				}

				UI::EndProperties();
			}
			UI::PopID();

			if (mySelectedSubMaterial->GetMaterialSpecializationData().IsValid())
			{
				ImGui::Separator();

				UI::Header("Material Parameters");

				UI::PushID();
				if (UI::BeginProperties("materialParams"))
				{
					auto& materialSpecializationParams = mySelectedSubMaterial->GetMaterialSpecializationData();

					for (auto& [name, memberData] : materialSpecializationParams.GetMembers())
					{
						if (auto it = std::find_if(textureDefinitions.begin(), textureDefinitions.end(), [&](const Volt::ShaderTexture& shaderTexture) { return shaderTexture.shaderName == name; }); it != textureDefinitions.end())
						{
							continue;
						}

						switch (memberData.type)
						{
							case Volt::ShaderUniformType::Bool: UI::Property(name, materialSpecializationParams.GetValue<bool>(name)); break;
							case Volt::ShaderUniformType::UInt:  UI::Property(name, materialSpecializationParams.GetValue<uint32_t>(name)); break;
							case Volt::ShaderUniformType::UInt2: UI::Property(name, materialSpecializationParams.GetValue<glm::uvec2>(name)); break;
							case Volt::ShaderUniformType::UInt3: UI::Property(name, materialSpecializationParams.GetValue<glm::uvec3>(name)); break;
							case Volt::ShaderUniformType::UInt4: UI::Property(name, materialSpecializationParams.GetValue<glm::uvec4>(name)); break;

							case Volt::ShaderUniformType::Int: UI::Property(name, materialSpecializationParams.GetValue<int32_t>(name)); break;
							case Volt::ShaderUniformType::Int2: UI::Property(name, materialSpecializationParams.GetValue<glm::ivec2>(name)); break;
							case Volt::ShaderUniformType::Int3: UI::Property(name, materialSpecializationParams.GetValue<glm::ivec3>(name)); break;
							case Volt::ShaderUniformType::Int4: UI::Property(name, materialSpecializationParams.GetValue<glm::ivec4>(name)); break;

							case Volt::ShaderUniformType::Float: UI::Property(name, materialSpecializationParams.GetValue<float>(name)); break;
							case Volt::ShaderUniformType::Float2: UI::Property(name, materialSpecializationParams.GetValue<glm::vec2>(name)); break;
							case Volt::ShaderUniformType::Float3: UI::Property(name, materialSpecializationParams.GetValue<glm::vec3>(name)); break;
							case Volt::ShaderUniformType::Float4: UI::Property(name, materialSpecializationParams.GetValue<glm::vec4>(name)); break;
						}
					}

					UI::EndProperties();
				}
				UI::PopID();
			}

			auto& pipelineGenerationDatas = mySelectedSubMaterial->GetPipelineGenerationDatas();

			if (!pipelineGenerationDatas.empty())
			{
				ImGui::Separator();

				UI::Header("Pipeline Generation");

				UI::PushID();
				for (auto& [stage, pipelineGenerationParams] : pipelineGenerationDatas)
				{
					if (pipelineGenerationParams.IsValid())
					{
						bool dataChanged = false;
						const uint32_t id = UI::GetID();

						if (UI::BeginProperties("pipelineGeneration" + std::to_string(id)))
						{
							for (auto& [name, memberData] : pipelineGenerationParams.GetMembers())
							{
								switch (memberData.type)
								{
									case Volt::ShaderUniformType::Bool: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<bool>(name)); break;
									case Volt::ShaderUniformType::UInt: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<uint32_t>(name)); break;
									case Volt::ShaderUniformType::UInt2: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::uvec2>(name)); break;
									case Volt::ShaderUniformType::UInt3: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::uvec3>(name)); break;
									case Volt::ShaderUniformType::UInt4: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::uvec4>(name)); break;

									case Volt::ShaderUniformType::Int: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<int32_t>(name)); break;
									case Volt::ShaderUniformType::Int2: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::ivec2>(name)); break;
									case Volt::ShaderUniformType::Int3: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::ivec3>(name)); break;
									case Volt::ShaderUniformType::Int4: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::ivec4>(name)); break;

									case Volt::ShaderUniformType::Float: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<float>(name)); break;
									case Volt::ShaderUniformType::Float2: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::vec2>(name)); break;
									case Volt::ShaderUniformType::Float3: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::vec3>(name)); break;
									case Volt::ShaderUniformType::Float4: dataChanged |= UI::Property(name, pipelineGenerationParams.GetValue<glm::vec4>(name)); break;
								}
							}

							UI::EndProperties();
						}

						if (dataChanged)
						{
							mySelectedSubMaterial->RecompilePermutation();
						}
					}
				}
				UI::PopID();
			}
		}
	}
	ImGui::End();
}

void MaterialEditorPanel::UpdatePreview()
{
	ImGui::SetNextWindowClass(GetWindowClass());

	if (ImGui::Begin("Preview##materialEditor", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		const auto size = ImGui::GetContentRegionAvail();
		myPreviewCamera->SetPerspectiveProjection(60.f, size.x / size.y, 1.f, 300.f);
		ImGui::Image(UI::GetTextureID(myPreviewRenderer->GetFinalImage()), size);
	}
	ImGui::End();
}

void MaterialEditorPanel::UpdateSubMaterials()
{
	using namespace AssetBrowser;

	ImGui::SetNextWindowClass(GetWindowClass());

	if (ImGui::Begin("Sub materials", nullptr, ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		if (mySelectedMaterial)
		{
			static float padding = 16.f;

			constexpr float thumbnailSize = 85.f;
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
				ImGui::PushID(std::string(material->GetName() + "##" + std::to_string(index)).c_str());

				const ImVec2 itemSize = AssetBrowserUtilities::GetBrowserItemSize(thumbnailSize);
				const float itemPadding = AssetBrowserUtilities::GetBrowserItemPadding();
				const ImVec2 minChild = AssetBrowserUtilities::GetBrowserItemPos();

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
						ImGui::BeginTooltip();
						ImGui::TextUnformatted(material->GetName().c_str());
						ImGui::EndTooltip();

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
							ImGui::Image(UI::GetTextureID(EditorResources::GetAssetIcon(Volt::AssetType::Material)), { thumbnailSize, thumbnailSize });
						}
						ImGui::EndChild();
						ImGui::PopStyleVar();

						UI::ShiftCursor(itemPadding / 2.f, 0.f);

						std::string materialName = material->GetName();
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - itemPadding / 2.f);
						if (ImGui::InputTextString(("##materialName" + std::to_string(index)).c_str(), &materialName))
						{
							material->SetName(materialName);
						}
						ImGui::PopItemWidth();

						//ImGui::TextWrapped(material->GetName().c_str());

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

			if (ImGui::BeginPopupContextWindow("rightClickMenu", ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Add Material"))
				{
					mySelectedMaterial->CreateSubMaterial(Volt::ShaderRegistry::GetShader("Illum"));
				}

				ImGui::EndPopup();
			}
		}
	}
	ImGui::End();
}

void MaterialEditorPanel::UpdateMaterials()
{
	ImGui::SetNextWindowClass(GetWindowClass());

	if (ImGui::Begin("Materials", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse))
	{
		ForceWindowDocked(ImGui::GetCurrentWindow());

		const auto& materials = Volt::AssetManager::GetAllAssetsOfType<Volt::Material>();

		if (EditorUtils::SearchBar(mySearchQuery, myHasSearchQuery))
		{
		}

		ImGui::BeginChild("Scrollable");
		{
			for (auto& material : materials)
			{
				const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(material);
				const std::string materialName = metadata.filePath.stem().string();
				if (myHasSearchQuery)
				{
					if (!Utility::StringContains(Utility::ToLower(materialName), Utility::ToLower(mySearchQuery)))
					{
						continue;
					}
				}

				bool selected = false;
				if (mySelectedMaterial)
				{
					selected = material == mySelectedMaterial->handle;
				}

				if (ImGui::Selectable(materialName.c_str(), &selected))
				{
					mySelectedMaterial = Volt::AssetManager::GetAsset<Volt::Material>(material);
					mySelectedSubMaterial = mySelectedMaterial->GetSubMaterials().at(0);

					myPreviewEntity.GetComponent<Volt::MeshComponent>().material = mySelectedMaterial->handle;
					myPreviewEntity.GetComponent<Volt::MeshComponent>().subMaterialIndex = 0;
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
