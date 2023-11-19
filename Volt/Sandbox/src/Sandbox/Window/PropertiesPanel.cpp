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

#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <GraphKey/Graph.h>

#include <vector>

PropertiesPanel::PropertiesPanel(Ref<Volt::Scene>& currentScene, Ref<Volt::SceneRendererNew>& currentSceneRenderer, SceneState& sceneState, const std::string& id)
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

		ComponentPropertyUtility::DrawComponents(myCurrentScene, entity);
		ComponentPropertyUtility::DrawMonoScripts(myCurrentScene, entity);
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
									comp.scriptIds.emplace_back();
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
								comp.scriptIds.emplace_back(UUID64());
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

						comp.scriptIds.emplace_back(UUID64());
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
						comp.scriptIds.emplace_back(UUID64());
						comp.scriptNames.emplace_back(fullMonoClassName);
					}
				}
			}
		}
	}
}
