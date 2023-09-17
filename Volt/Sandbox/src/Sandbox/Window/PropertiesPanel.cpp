#include "sbpch.h"
#include "PropertiesPanel.h"

#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/ComponentPropertyUtilities.h"
#include "Sandbox/Window/GraphKey/GraphKeyPanel.h"

#include "Sandbox/Sandbox.h"
#include "Sandbox/UserSettingsManager.h"
#include "Sandbox/EditorCommandStack.h"

#include <Volt/Components/LightComponents.h>

#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/MouseButtonCodes.h>
#include <Volt/Input/Input.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/StringUtility.h>
#include <Volt/Utility/PremadeCommands.h>

#include <Volt/Scripting/Mono/MonoScriptInstance.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Scripting/Mono/MonoScriptClass.h>
#include <Volt/Scripting/Mono/MonoScriptUtils.h>
#include <Volt/Scripting/Mono/MonoEnum.h>

#include <Volt/ImGui/ImGuiImplementation.h>

#include <GraphKey/Graph.h>

#include <vector>

namespace Utility
{
	inline static Volt::AssetType AssetTypeFromMonoType(Volt::MonoFieldType type)
	{
		switch (type)
		{
			case Volt::MonoFieldType::Animation: return Volt::AssetType::Animation;
				break;
			case Volt::MonoFieldType::Prefab: return Volt::AssetType::Prefab;
				break;
			case Volt::MonoFieldType::Scene: return Volt::AssetType::Scene;
				break;
			case Volt::MonoFieldType::Mesh: return Volt::AssetType::Mesh;
				break;
			case Volt::MonoFieldType::Font: return Volt::AssetType::Font;
				break;
			case Volt::MonoFieldType::Material: return Volt::AssetType::Material;
				break;
			case Volt::MonoFieldType::Texture: return Volt::AssetType::Texture;
				break;
			case Volt::MonoFieldType::PostProcessingMaterial: return Volt::AssetType::PostProcessingMaterial;
				break;
			case Volt::MonoFieldType::Video: return Volt::AssetType::Video;
				break;
			case Volt::MonoFieldType::AnimationGraph: return Volt::AssetType::AnimationGraph;
		}

		return Volt::AssetType::None;
	}
}

PropertiesPanel::PropertiesPanel(Ref<Volt::Scene>& currentScene, Ref<Volt::SceneRenderer>& currentSceneRenderer, SceneState& sceneState, const std::string& id)
	: EditorWindow("Properties", false, id), myCurrentScene(currentScene), myCurrentSceneRenderer(currentSceneRenderer), mySceneState(sceneState)
{
	m_isOpen = true;
	myMaxEventListSize = 20;
	myLastValue = std::make_shared<PropertyEvent>();
}

void PropertiesPanel::UpdateMainContent()
{
	if (myMidEvent == true)
	{
		if (ImGui::IsMouseReleased(0))
		{
			myMidEvent = false;
		}
	}

	if (!SelectionManager::IsAnySelected())
	{
		return;
	}

	const bool singleSelected = !(SelectionManager::GetSelectedCount() > 1);
	auto firstEntity = Volt::Entity{ SelectionManager::GetSelectedEntities().front(), myCurrentScene };
	const auto& entities = SelectionManager::GetSelectedEntities();

	if (singleSelected)
	{
		if (firstEntity.HasComponent<Volt::TagComponent>())
		{
			auto& tag = firstEntity.GetComponent<Volt::TagComponent>();
			UI::InputText("Name", tag.tag);
		}
	}
	else
	{
		static std::string inputText = "...";

		std::string firstName;
		bool sameName = true;

		if (firstEntity.HasComponent<Volt::TagComponent>())
		{
			firstName = firstEntity.GetTag();
		}

		for (auto& id : SelectionManager::GetSelectedEntities())
		{
			Volt::Entity entity{ id, myCurrentScene };

			if (entity.HasComponent<Volt::TagComponent>())
			{
				auto& tag = entity.GetComponent<Volt::TagComponent>();
				if (tag.tag != firstName)
				{
					sameName = false;
					break;
				}
			}
		}

		if (sameName)
		{
			inputText = firstName;
		}
		else
		{
			inputText = "...";
		}

		UI::PushID();
		if (UI::InputText("Name", inputText))
		{
			for (auto& id : SelectionManager::GetSelectedEntities())
			{
				Volt::Entity entity{ id, myCurrentScene };

				if (entity.HasComponent<Volt::TagComponent>())
				{
					entity.GetComponent<Volt::TagComponent>().tag = inputText;
				}
			}
		}
		UI::PopID();
	}

	// Transform
	{
		UI::PushID();
		if (UI::BeginProperties("Transform"))
		{
			auto& entityId = SelectionManager::GetSelectedEntities().front();
			Volt::Entity entity{ entityId, myCurrentScene };

			if (entity.HasComponent<Volt::TransformComponent>())
			{
				static bool shouldUpdateNavMesh = false;

				auto& transform = entity.GetComponent<Volt::TransformComponent>();

				if (UI::PropertyAxisColor("Position", transform.position, 0.f))
				{
					shouldUpdateNavMesh = true;

					if (myMidEvent == false)
					{
						Ref<ValueCommand<glm::vec3>> command = CreateRef<ValueCommand<glm::vec3>>(&transform.position, transform.position);
						EditorCommandStack::PushUndo(command);
						myMidEvent = true;
					}

					for (auto& entId : entities)
					{
						Volt::Entity ent{ entId, myCurrentScene.get() };
						ent.SetLocalPosition(transform.position);
						myCurrentScene->InvalidateEntityTransform(entId);
					}
				}

				const glm::vec3 originalEuler = glm::eulerAngles(transform.rotation);
				glm::vec3 rotDegrees = glm::degrees(originalEuler);

				if (UI::PropertyAxisColor("Rotation", rotDegrees, 0.f))
				{
					shouldUpdateNavMesh = true;
					transform.rotation = glm::quat{ glm::radians(rotDegrees) };

					if (myMidEvent == false)
					{
						Ref<ValueCommand<glm::quat>> command = CreateRef<ValueCommand<glm::quat>>(&transform.rotation, transform.rotation);
						EditorCommandStack::PushUndo(command);
						myMidEvent = true;
					}

					for (auto& entId : entities)
					{
						Volt::Entity ent{ entId, myCurrentScene.get() };
						ent.SetLocalRotation(transform.rotation);
						myCurrentScene->InvalidateEntityTransform(entId);
					}
				}

				if (UI::PropertyAxisColor("Scale", transform.scale, 1.f))
				{
					shouldUpdateNavMesh = true;

					if (myMidEvent == false)
					{
						Ref<ValueCommand<glm::vec3>> command = CreateRef<ValueCommand<glm::vec3>>(&transform.scale, transform.scale);
						EditorCommandStack::PushUndo(command);
						myMidEvent = true;
					}

					for (auto& entId : entities)
					{
						Volt::Entity ent{ entId, myCurrentScene.get() };
						ent.SetLocalScale(transform.scale);
						myCurrentScene->InvalidateEntityTransform(entId);
					}
				}

				if (shouldUpdateNavMesh && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && Sandbox::Get().CheckForUpdateNavMesh(entity))
				{
					Sandbox::Get().BakeNavMesh();
					shouldUpdateNavMesh = false;
				}
			}

			UI::EndProperties();
		}
		UI::PopID();
	}

	if (singleSelected)
	{
		const auto id = SelectionManager::GetSelectedEntities().front();
		Volt::Entity entity{ id, myCurrentScene };

		ComponentPropertyUtility::DrawComponentProperties(myCurrentScene, entity);
	}

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.2f, 0.2f, 1.f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);

	if (ImGui::BeginTable("addTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit))
	{
		const float width = ImGui::GetContentRegionAvail().x / 2.f;
		constexpr float buttonHeight = 22.f;

		ImGui::TableSetupColumn("Column1", 0, width);
		ImGui::TableSetupColumn("Column2", 0, width);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("Add Component", { width, buttonHeight }))
		{
			myComponentSearchQuery = "";
			myActivateComponentSearch = true;
			UI::OpenPopup("AddComponent");
		}

		ImGui::TableNextColumn();

		if (ImGui::Button("Add Script", { width, buttonHeight }))
		{
			myScriptSearchQuery = "";
			myActivateScriptSearch = true;
			UI::OpenPopup("AddMonoScript");
		}

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	AddComponentPopup();
	AddMonoScriptPopup();
	AcceptMonoDragDrop();
}

void PropertiesPanel::AddComponentPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (UI::BeginPopup("AddComponent" + m_id, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		std::vector<std::string> componentNames;
		std::unordered_map<std::string, VoltGUID> nameToGUIDMap;

		const auto& componentRegistry = Volt::ComponentRegistry::GetRegistry();
		for (const auto& [guid, typeDesc] : componentRegistry)
		{
			if (typeDesc->GetValueType() == Volt::ValueType::Component)
			{
				const Volt::IComponentTypeDesc* compDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);
				if (compDesc->IsHidden())
				{
					continue;
				}
				
				const std::string strLabel = std::string(compDesc->GetLabel());
				
				componentNames.emplace_back(strLabel);
				nameToGUIDMap[strLabel] = guid;
			}
		}

		std::sort(componentNames.begin(), componentNames.end());

		// Search bar
		{
			bool t;
			EditorUtils::SearchBar(myComponentSearchQuery, t, myActivateComponentSearch);
			if (myActivateComponentSearch)
			{
				myActivateComponentSearch = false;
			}
		}

		if (!myComponentSearchQuery.empty())
		{
			componentNames = UI::GetEntriesMatchingQuery(myComponentSearchQuery, componentNames);
		}

		{
			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			for (const auto& label : componentNames)
			{
				const auto compGuid = nameToGUIDMap.at(label);
				std::string_view componentTypeName = Volt::ComponentRegistry::GetTypeNameFromGUID(compGuid);

				const bool newMonoScript = compGuid == Volt::MonoScriptComponent::guid;

				Volt::Entity frontEntity{ SelectionManager::GetSelectedEntities().front(), myCurrentScene };
				if (!frontEntity.HasComponent(componentTypeName) || newMonoScript)
				{
					UI::ShiftCursor(4.f, 0.f);
					UI::RenderMatchingTextBackground(myComponentSearchQuery, label, EditorTheme::MatchingTextBackground);
					if (ImGui::MenuItem(label.c_str()))
					{
						for (auto& ent : SelectionManager::GetSelectedEntities())
						{
							if (!Volt::ComponentRegistry::Helpers::HasComponentWithGUID(compGuid, myCurrentScene->GetRegistry(), ent))
							{
								Volt::ComponentRegistry::Helpers::AddComponentWithGUID(compGuid, myCurrentScene->GetRegistry(), ent);
							}

							if (newMonoScript)
							{
								Volt::MonoScriptComponent& comp = myCurrentScene->GetRegistry().get<Volt::MonoScriptComponent>(frontEntity.GetID());
								if (comp.scriptIds.size() < Volt::MonoScriptEngine::MAX_SCRIPTS_PER_ENTITY)
								{
									comp.scriptIds.emplace_back(Volt::UUID());
									comp.scriptNames.emplace_back("");
								}
							}
						}

						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::EndChild();
		}

		UI::EndPopup();
	}
}

void PropertiesPanel::AddMonoScriptPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (UI::BeginPopup("AddMonoScript" + m_id))
	{
		const auto& scriptInfo = Volt::MonoScriptEngine::GetRegisteredClasses();

		std::vector<std::string> scriptNames;
		std::vector<std::string> fullScriptNames;

		for (const auto& klass : scriptInfo)
		{
			auto classname = Volt::MonoScriptUtils::GetClassName(klass.first);
			classname[0] = static_cast<char>(std::toupper(classname[0]));
			scriptNames.emplace_back(classname);
			fullScriptNames.emplace_back(klass.first);
		}

		std::sort(scriptNames.begin(), scriptNames.end());

		// Search bar
		{
			bool t;
			EditorUtils::SearchBar(myScriptSearchQuery, t, myActivateScriptSearch);
			if (myActivateScriptSearch)
			{
				myActivateScriptSearch = false;
			}
		}

		if (!myScriptSearchQuery.empty())
		{
			scriptNames = UI::GetEntriesMatchingQuery(myScriptSearchQuery, scriptNames);
		}

		bool addedScript = false;

		{
			UI::ScopedColor background{ ImGuiCol_ChildBg, EditorTheme::DarkGreyBackground };
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			for (const auto& name : scriptNames)
			{
				UI::ShiftCursor(4.f, 0.f);
				UI::RenderMatchingTextBackground(myScriptSearchQuery, name, EditorTheme::MatchingTextBackground);
				if (ImGui::MenuItem(name.c_str()))
				{
					for (auto& id : SelectionManager::GetSelectedEntities())
					{
						Volt::Entity entity{ id, myCurrentScene };

						if (!entity.HasComponent<Volt::MonoScriptComponent>())
						{
							entity.AddComponent<Volt::MonoScriptComponent>();
						}

						auto& comp = entity.GetComponent<Volt::MonoScriptComponent>();
						if (comp.scriptIds.size() < Volt::MonoScriptEngine::MAX_SCRIPTS_PER_ENTITY)
						{
							std::string fullScriptName = "";

							for (const auto& scriptName : fullScriptNames)
							{
								size_t lastDotPos = scriptName.find_last_of(".");
								std::string aSubstr = scriptName.substr(lastDotPos + 1);

								if (aSubstr == name)
								{
									fullScriptName = scriptName;
								}
							}

							if (!fullScriptName.empty())
							{
								comp.scriptIds.emplace_back(Volt::UUID());
								comp.scriptNames.emplace_back(fullScriptName);
							}
						}
					}
					ImGui::CloseCurrentPopup();

					addedScript = true;
				}
			}

			if (!myScriptSearchQuery.empty() && ImGui::MenuItem(("Create " + myScriptSearchQuery + " script").c_str()))
			{
				UI::ShiftCursor(4.f, 0.f);

				Volt::MonoScriptUtils::CreateNewCSFile(myScriptSearchQuery, "", true);

				for (auto& id : SelectionManager::GetSelectedEntities())
				{
					Volt::Entity entity{ id, myCurrentScene };

					if (!entity.HasComponent<Volt::MonoScriptComponent>())
					{
						entity.AddComponent<Volt::MonoScriptComponent>(id);
					}

					auto& comp = entity.GetComponent<Volt::MonoScriptComponent>();
					if (comp.scriptIds.size() < Volt::MonoScriptEngine::MAX_SCRIPTS_PER_ENTITY)
					{
						std::string fullScriptName = "Project." + myScriptSearchQuery;

						comp.scriptIds.emplace_back(Volt::UUID());
						comp.scriptNames.emplace_back(fullScriptName);
					}
				}
				ImGui::CloseCurrentPopup();

				addedScript = true;
			}

			ImGui::EndChild();
		}

		if (addedScript && mySceneState == SceneState::Edit)
		{
			myCurrentScene->ShutdownEngineScripts();
			myCurrentScene->InitializeEngineScripts();
		}

		UI::EndPopup();
	}
}

void PropertiesPanel::AcceptMonoDragDrop()
{
	ImGui::Dummy(ImGui::GetContentRegionAvail());
	if (void* ptr = UI::DragDropTarget({ "ASSET_BROWSER_ITEM" }))
	{
		Volt::AssetHandle handle = *(Volt::AssetHandle*)ptr;
		auto path = Volt::AssetManager::Get().GetFilesystemPath(handle);
		if (path.extension() == ".cs")
		{
			auto name = path.filename();
			name.replace_extension("");

			std::string fullMonoClassName;
			for (const auto& klass : Volt::MonoScriptEngine::GetRegisteredClasses())
			{
				auto className = klass.first;
				auto pos = className.find(".");
				while (pos != std::string::npos)
				{
					className = className.substr(pos + 1, className.size());
					pos = className.find('.');
				}

				if (name == className)
				{
					fullMonoClassName = klass.first;
					break;
				}
			}

			if (!fullMonoClassName.empty())
			{
				for (auto& id : SelectionManager::GetSelectedEntities())
				{
					Volt::Entity entity{ id, myCurrentScene };

					if (!entity.HasComponent<Volt::MonoScriptComponent>())
					{
						entity.AddComponent<Volt::MonoScriptComponent>();
					}

					auto& comp = entity.GetComponent<Volt::MonoScriptComponent>();
					if (comp.scriptIds.size() < Volt::MonoScriptEngine::MAX_SCRIPTS_PER_ENTITY)
					{
						comp.scriptIds.emplace_back(Volt::UUID());
						comp.scriptNames.emplace_back(fullMonoClassName);
					}
				}
			}
		}
	}
}

void PropertiesPanel::DrawMonoScript(Volt::MonoScriptEntry& scriptEntry, const entt::entity& entId)
{
	Volt::Entity entity{ entId, myCurrentScene };

	std::string scriptClassName = Volt::MonoScriptUtils::GetClassName(scriptEntry.name);
	scriptClassName[0] = static_cast<char>(std::toupper(scriptClassName[0]));

	bool removeComp = false;
	bool open = UI::CollapsingHeader(scriptClassName + " Script");
	float buttonSize = 22.f + GImGui->Style.FramePadding.y * 0.5f;
	float availRegion = ImGui::GetContentRegionAvail().x;

	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
	{
		if (ImGui::Button("Open in Visual Studio"))
		{
			auto path = Volt::AssetManager::GetFilePathFromFilename(scriptClassName + ".cs");
			if (!Volt::PremadeCommands::RunOpenVSFileCommand(UserSettingsManager::GetSettings().externalToolsSettings.customExternalScriptEditor, path))
			{
				UI::Notify(NotificationType::Error, "Open file failed!", "External script editor is not valid!");
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (!open)
	{
		UI::SameLine(availRegion - buttonSize * 0.5f);
	}
	else
	{
		UI::SameLine(availRegion + buttonSize * 0.5f);
	}
	std::string id = "-###Remove" + std::format("{0}", (uint64_t)scriptEntry.id);

	{
		UI::ScopedStyleFloat round{ ImGuiStyleVar_FrameRounding, 0.f };
		UI::ScopedStyleFloat2 pad{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		UI::ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
		UI::ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
		UI::ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

		if (ImGui::Button(id.c_str(), ImVec2{ buttonSize, buttonSize }))
		{
			removeComp = true;
		}
	}

	if (open)
	{
		UI::PushID();

		DrawMonoProperties(scriptEntry);

		UI::PopID();
	}

	if (removeComp)
	{
		auto& scriptComp = entity.GetComponent<Volt::MonoScriptComponent>();

		for (uint32_t i = 0; i < scriptComp.scriptIds.size(); ++i)
		{
			if (scriptComp.scriptIds[i] == scriptEntry.id)
			{
				scriptComp.scriptIds.erase(scriptComp.scriptIds.begin() + i);
				scriptComp.scriptNames.erase(scriptComp.scriptNames.begin() + i);
				break;
			}
		}
		if (scriptComp.scriptIds.empty())
		{
			entity.RemoveComponent<Volt::MonoScriptComponent>();
		}

		if (mySceneState == SceneState::Edit)
		{
			myCurrentScene->ShutdownEngineScripts();
			myCurrentScene->InitializeEngineScripts();
		}
	}
}

void PropertiesPanel::DrawMonoProperties(Volt::MonoScriptEntry& scriptEntry)
{
	Ref<Volt::MonoScriptInstance> scriptInstance = Volt::MonoScriptEngine::GetInstanceFromId(scriptEntry.id);
	if (UI::BeginProperties("MonoScripts"))
	{
		if ((myCurrentScene->IsPlaying()) && scriptInstance)
		{
			for (const auto& [name, field] : scriptInstance->GetClass()->GetFields())
			{
				std::string displayName = name;
				if (field.netData.replicatedCondition == Volt::eRepCondition::CONTINUOUS) displayName = "[C] " + displayName;
				else if (field.netData.replicatedCondition == Volt::eRepCondition::NOTIFY) displayName = "[N] " + displayName;

				switch (field.type)
				{
					case Volt::MonoFieldType::String:
					{
						std::string value = scriptInstance->GetField<std::string>(name);
						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, value);
						}

						break;
					}

					case Volt::MonoFieldType::Bool:
					{
						bool value = scriptInstance->GetField<bool>(name);
						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Int:
					{
						int32_t value = scriptInstance->GetField<int32_t>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::UInt:
					{
						uint32_t value = scriptInstance->GetField<uint32_t>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Short:
					{
						int16_t value = scriptInstance->GetField<int16_t>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::UShort:
					{
						uint16_t value = scriptInstance->GetField<uint16_t>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Char:
					{
						int8_t value = scriptInstance->GetField<int8_t>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::UChar:
					{
						uint8_t value = scriptInstance->GetField<uint8_t>(name);
						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::Float:
					{
						float value = scriptInstance->GetField<float>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Double:
					{
						double value = scriptInstance->GetField<double>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::Vector2:
					{
						glm::vec2 value = scriptInstance->GetField<glm::vec2>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::Vector3:
					{
						glm::vec3 value = scriptInstance->GetField<glm::vec3>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::Vector4:
					{
						glm::vec4 value = scriptInstance->GetField<glm::vec4>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Quaternion:
					{
						glm::vec4 value = scriptInstance->GetField<glm::vec4>(name);

						if (UI::Property(displayName, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Volt::MonoFieldType::Animation:
					case Volt::MonoFieldType::Prefab:
					case Volt::MonoFieldType::Scene:
					case Volt::MonoFieldType::Font:
					case Volt::MonoFieldType::Mesh:
					case Volt::MonoFieldType::Material:
					case Volt::MonoFieldType::Video:
					case Volt::MonoFieldType::AnimationGraph:
					case Volt::MonoFieldType::Texture:
					{
						Volt::AssetHandle value = scriptInstance->GetField<Volt::AssetHandle>(name);

						if (EditorUtils::Property(displayName, value, Utility::AssetTypeFromMonoType(field.type)))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Volt::MonoFieldType::Enum:
					{
						int32_t enumVal = scriptInstance->GetField<uint32_t>(name);

						auto& entityFields = myCurrentScene->GetScriptFieldCache().GetCache()[scriptEntry.id];
						const auto enumName = entityFields[name]->field.enumName;

						const auto& enumData = Volt::MonoScriptEngine::GetRegisteredEnums().at(enumName);
						const auto& enumValues = enumData->GetValues();

						std::vector<std::string> valueNames{}; // #TODO_Ivar: Add support for non linear enum values
						for (const auto& [valueName, val] : enumValues)
						{
							valueNames.emplace_back(valueName);
						}

						if (UI::ComboProperty(displayName, enumVal, valueNames))
						{
							scriptInstance->SetField(name, &enumVal);
						}
					}
				}
			}
		}
		else
		{
			if (Volt::MonoScriptEngine::EntityClassExists(scriptEntry.name))
			{
				const auto& classFields = Volt::MonoScriptEngine::GetScriptClass(scriptEntry.name)->GetFields();
				const auto& entDefFieldMap = Volt::MonoScriptEngine::GetDefaultScriptFieldMap(scriptEntry.name);

				auto& entityFields = myCurrentScene->GetScriptFieldCache().GetCache()[scriptEntry.id];

				for (const auto& [name, field] : classFields)
				{
					if (!entityFields.contains(name))
					{
						Ref<Volt::MonoScriptFieldInstance> instance = CreateRef<Volt::MonoScriptFieldInstance>();
						instance->field = field;
						instance->data.Allocate(entDefFieldMap.at(name)->data.GetSize());
						instance->data.Copy(entDefFieldMap.at(name)->data.As<void>(), entDefFieldMap.at(name)->data.GetSize());
						entityFields[name] = instance;
					}

					if (entityFields.contains(name))
					{
						std::string displayName = name;
						if (field.netData.replicatedCondition == Volt::eRepCondition::CONTINUOUS) displayName = "[C] " + displayName;
						else if (field.netData.replicatedCondition == Volt::eRepCondition::NOTIFY) displayName = "[N] " + displayName;

						auto& entField = entityFields.at(name);

						bool fontChanged = false;

						if (entField->data.IsValid() && !entDefFieldMap.at(name)->data.IsValid())
						{
							fontChanged = true;
							UI::PushFont(FontType::Bold_17);
						}
						else if (memcmp(entField->data.As<void>(), entDefFieldMap.at(name)->data.As<void>(), entField->data.GetSize()) != 0)
						{
							fontChanged = true;
							UI::PushFont(FontType::Bold_17);
						}

						bool fieldChanged = false;

						switch (field.type)
						{
							case Volt::MonoFieldType::Bool: fieldChanged = UI::Property(name, *entField->data.As<bool>()); break;
							case Volt::MonoFieldType::String:
							{
								std::string str;

								if (entField->data.IsValid())
								{
									str = std::string(entField->data.As<const char>());
								}

								if (UI::Property(name, str))
								{
									entField->SetValue(str, str.size(), field.type);
									if (scriptInstance)
									{
										scriptInstance->SetField(name, str);
									}
								}
								break;
							}

							case Volt::MonoFieldType::Int: fieldChanged = UI::Property(displayName, *entField->data.As<int32_t>());
								break;
							case Volt::MonoFieldType::UInt: fieldChanged = UI::Property(displayName, *entField->data.As<uint32_t>());
								break;

							case Volt::MonoFieldType::Short: fieldChanged = UI::Property(displayName, *entField->data.As<int16_t>());
								break;
							case Volt::MonoFieldType::UShort: fieldChanged = UI::Property(displayName, *entField->data.As<uint16_t>());
								break;

							case Volt::MonoFieldType::Char: fieldChanged = UI::Property(displayName, *entField->data.As<int8_t>());
								break;
							case Volt::MonoFieldType::UChar: fieldChanged = UI::Property(displayName, *entField->data.As<uint8_t>());
								break;

							case Volt::MonoFieldType::Float: fieldChanged = UI::Property(displayName, *entField->data.As<float>());
								break;
							case Volt::MonoFieldType::Double: fieldChanged = UI::Property(displayName, *entField->data.As<double>());
								break;

							case Volt::MonoFieldType::Vector2: fieldChanged = UI::Property(displayName, *entField->data.As<glm::vec2>());
								break;
							case Volt::MonoFieldType::Vector3: fieldChanged = UI::Property(displayName, *entField->data.As<glm::vec3>());
								break;
							case Volt::MonoFieldType::Vector4: fieldChanged = UI::Property(displayName, *entField->data.As<glm::vec4>());
								break;
							case Volt::MonoFieldType::Quaternion: fieldChanged = UI::Property(displayName, *entField->data.As<glm::vec4>());
								break;

							case Volt::MonoFieldType::Animation: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Animation);
								break;
							case Volt::MonoFieldType::Prefab: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Prefab);
								break;
							case Volt::MonoFieldType::Scene: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Scene);
								break;
							case Volt::MonoFieldType::Font: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Font);
								break;
							case Volt::MonoFieldType::Mesh: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Mesh);
								break;
							case Volt::MonoFieldType::Material: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Material);
								break;
							case Volt::MonoFieldType::Texture: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Texture);
								break;
							case Volt::MonoFieldType::PostProcessingMaterial: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::PostProcessingMaterial);
								break;
							case Volt::MonoFieldType::Video: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::Video);
								break;
							case Volt::MonoFieldType::Asset: fieldChanged = EditorUtils::Property(displayName, *entField->data.As<Volt::AssetHandle>(), Volt::AssetType::None);
								break;

							case Volt::MonoFieldType::Entity: fieldChanged = UI::PropertyEntity(displayName, myCurrentScene, *entField->data.As<entt::entity>());
								break;

							case Volt::MonoFieldType::Color: fieldChanged = UI::PropertyColor(displayName, *entField->data.As<glm::vec4>());
								break;
							case Volt::MonoFieldType::Enum:
							{
								int32_t& enumVal = (int32_t&)*entField->data.As<uint32_t>();
								const auto enumName = entityFields[name]->field.enumName;

								const auto& enumData = Volt::MonoScriptEngine::GetRegisteredEnums().at(enumName);
								const auto& enumValues = enumData->GetValues();

								std::vector<std::string> valueNames{}; // #TODO_Ivar: Add support for non linear enum values
								for (const auto& [valueName, val] : enumValues)
								{
									valueNames.emplace_back(valueName);
								}

								fieldChanged = UI::ComboProperty(displayName, enumVal, valueNames);
							}
						}

						if (fieldChanged && scriptInstance)
						{
							scriptInstance->SetField(name, entField->data.As<void>());
						}

						if (fontChanged)
						{
							UI::PopFont();
						}

						auto id = UI::GetID();
						std::string strId = "##" + name + std::to_string(id);

						if (ImGui::BeginPopupContextItem(strId.c_str(), ImGuiPopupFlags_MouseButtonRight))
						{
							ImGui::PushStyleColor(ImGuiCol_Button, { 1.f, 1.f, 1.f, 0.f });
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.f, 1.f, 1.f, 0.f });
							if (ImGui::Button("Reset Value"))
							{
								entityFields.erase(name);
								ImGui::CloseCurrentPopup();
							}
							ImGui::PopStyleColor();
							ImGui::PopStyleColor();
							ImGui::EndPopup();
						}
					}
				}
			}
		}
		UI::EndProperties();
	}
}
