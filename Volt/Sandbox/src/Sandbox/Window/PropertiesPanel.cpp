#include "sbpch.h"
#include "PropertiesPanel.h"

#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/EditorUtilities.h"
#include "Sandbox/Utility/EditorIconLibrary.h"

#include <Volt/Utility/UIUtility.h>
#include <Volt/Scripting/ScriptRegistry.h>
#include <Volt/Scripting/ScriptEngine.h>
#include <Volt/Scripting/ScriptBase.h>

#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/MouseButtonCodes.h>
#include <Volt/Input/Input.h>
#include <Volt/Utility/StringUtility.h>

#include <Volt/Scripting/Mono/MonoScriptInstance.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Scripting/Mono/MonoScriptClass.h>

#include <Wire/Serialization.h>

#include <vector>

#include "Sandbox/EditorCommandStack.h"

PropertiesPanel::PropertiesPanel(Ref<Volt::Scene>& currentScene)
	: EditorWindow("Properties"), myCurrentScene(currentScene)
{
	myIsOpen = true;
	myMaxEventListSize = 20;
	myLastValue = std::make_shared<PropertyEvent>();
}

void PropertiesPanel::UpdateMainContent()
{
	if (myMidEvent == true)
	{
		if (Volt::Input::IsMouseButtonReleased(VT_MOUSE_BUTTON_LEFT))
		{

			myMidEvent = false;
		}
	}

	UI::ScopedStyleFloat rounding{ ImGuiStyleVar_FrameRounding, 2.f };

	if (!SelectionManager::IsAnySelected())
	{
		return;
	}

	const bool singleSelected = !(SelectionManager::GetSelectedCount() > 1);
	auto& registry = myCurrentScene->GetRegistry();
	const auto firstEntity = SelectionManager::GetSelectedEntities().front();
	const auto entities = SelectionManager::GetSelectedEntities();

	if (singleSelected)
	{
		if (registry.HasComponent<Volt::TagComponent>(firstEntity))
		{
			auto& tag = registry.GetComponent<Volt::TagComponent>(firstEntity);
			UI::InputText("Name", tag.tag);
		}
	}
	else
	{
		static std::string inputText = "...";

		std::string firstName;
		bool sameName = true;

		if (registry.HasComponent<Volt::TagComponent>(firstEntity))
		{
			firstName = registry.GetComponent<Volt::TagComponent>(firstEntity).tag;
		}

		for (auto& entity : SelectionManager::GetSelectedEntities())
		{
			if (registry.HasComponent<Volt::TagComponent>(entity))
			{
				auto& tag = registry.GetComponent<Volt::TagComponent>(entity);
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

		UI::PushId();
		if (UI::InputText("Name", inputText))
		{
			for (auto& entity : SelectionManager::GetSelectedEntities())
			{
				if (registry.HasComponent<Volt::TagComponent>(entity))
				{
					registry.GetComponent<Volt::TagComponent>(entity).tag = inputText;
				}
			}
		}
		UI::PopId();
	}

	// Transform
	{
		UI::PushId();
		if (UI::BeginProperties("Transform"))
		{
			auto& entity = SelectionManager::GetSelectedEntities().front();

			if (registry.HasComponent<Volt::TransformComponent>(entity))
			{
				auto& transform = registry.GetComponent<Volt::TransformComponent>(entity); // #SAMUEL_TODO: Currently this displays local space if parented.

				if (UI::PropertyAxisColor("Position", transform.position, 0.f, (singleSelected) ? std::function<void(gem::vec3&)>() : [&](gem::vec3& val)
					{
						for (auto& ent : entities)
						{
							auto& entTransform = registry.GetComponent<Volt::TransformComponent>(ent);
							entTransform.position = val;
						}
					}))
				{
					if (myMidEvent == false)
					{
						Ref<ValueCommand<gem::vec3>> command = CreateRef<ValueCommand<gem::vec3>>(&transform.position, transform.position);
						EditorCommandStack::PushUndo(command);
						myMidEvent = true;
					}
				}

					gem::vec3 rotDegrees = gem::degrees(gem::eulerAngles(transform.rotation));
					if (UI::PropertyAxisColor("Rotation", rotDegrees, 0.f, (singleSelected) ? std::function<void(gem::vec3&)>() : [&](gem::vec3& val)
						{
							for (auto& ent : entities)
							{
								auto& entTransform = registry.GetComponent<Volt::TransformComponent>(ent);
								entTransform.rotation = gem::radians(val);
							}
						}))
					{
						transform.rotation = gem::quat{ gem::radians(rotDegrees) };

						if (myMidEvent == false)
						{
							Ref<ValueCommand<gem::quat>> command = CreateRef<ValueCommand<gem::quat>>(&transform.rotation, transform.rotation);
							EditorCommandStack::PushUndo(command);
							myMidEvent = true;
						}
					}

						if (UI::PropertyAxisColor("Scale", transform.scale, 1.f, (singleSelected) ? std::function<void(gem::vec3&)>() : [&](gem::vec3& val)
							{
								for (auto& ent : entities)
								{
									auto& entTransform = registry.GetComponent<Volt::TransformComponent>(ent);
									entTransform.scale = val;
								}
							}))
						{
							if (myMidEvent == false)
							{
								Ref<ValueCommand<gem::vec3>> command = CreateRef<ValueCommand<gem::vec3>>(&transform.scale, transform.scale);
								EditorCommandStack::PushUndo(command);
								myMidEvent = true;
							}
						}
			}

			UI::EndProperties();
		}
		UI::PopId();
	}

	if (singleSelected)
	{
		const auto entity = SelectionManager::GetSelectedEntities().front();

		for (const auto& [guid, pool] : registry.GetPools())
		{
			if (!registry.HasComponent(guid, entity))
			{
				continue;
			}

			const auto& registryInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
			if (registryInfo.name == "TagComponent" || registryInfo.name == "TransformComponent" || registryInfo.name == "RelationshipComponent" || registryInfo.name == "PrefabComponent" ||
				registryInfo.name == "EntityDataComponent")
			{
				continue;
			}

			bool removeComp = false;
			bool open = UI::TreeNodeFramed(registryInfo.name, true, 2.f);
			float buttonSize = 21.f + GImGui->Style.FramePadding.y;
			float availRegion = ImGui::GetContentRegionAvail().x;

			if (!open)
			{
				UI::SameLine(availRegion - buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
			}
			else
			{
				UI::SameLine(availRegion + buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
			}
			std::string id = "-###Remove" + registryInfo.name;

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
				UI::PushId();
				if (registryInfo.name != "ScriptComponent" && registryInfo.name != "MonoScriptComponent" && UI::BeginProperties(registryInfo.name))
				{
					uint8_t* data = (uint8_t*)registry.GetComponentPtr(guid, entity);
					for (auto& prop : registryInfo.properties)
					{
						if (!prop.visible)
						{
							continue;
						}

						switch (prop.type)
						{
							case Wire::ComponentRegistry::PropertyType::Bool: UI::Property(prop.name, *(bool*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::String: UI::Property(prop.name, *(std::string*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Int: UI::Property(prop.name, *(int32_t*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::UInt: UI::Property(prop.name, *(uint32_t*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Short: UI::Property(prop.name, *(int16_t*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::UShort: UI::Property(prop.name, *(uint16_t*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Char: UI::Property(prop.name, *(int8_t*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::UChar: UI::Property(prop.name, *(uint8_t*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Float: UI::Property(prop.name, *(float*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Double: UI::Property(prop.name, *(double*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Vector2: UI::Property(prop.name, *(gem::vec2*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Vector3: UI::Property(prop.name, *(gem::vec3*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Vector4: UI::Property(prop.name, *(gem::vec4*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Quaternion: UI::Property(prop.name, *(gem::vec4*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::EntityId: UI::PropertyEntity(prop.name, myCurrentScene, *(Wire::EntityId*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Color3: UI::PropertyColor(prop.name, *(gem::vec3*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Color4: UI::PropertyColor(prop.name, *(gem::vec4*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::AssetHandle:
							{
								Volt::AssetType assetType = Volt::AssetType::None;

								if (prop.specialType.has_value())
								{
									assetType = std::any_cast<Volt::AssetType>(prop.specialType);
								}

								EditorUtils::Property(prop.name, *(Volt::AssetHandle*)(&data[prop.offset]), assetType);
								break;
							}

							case Wire::ComponentRegistry::PropertyType::Folder: UI::PropertyDirectory(prop.name, *(std::filesystem::path*)(&data[prop.offset])); break;
							case Wire::ComponentRegistry::PropertyType::Path: UI::Property(prop.name, *(std::filesystem::path*)(&data[prop.offset])); break;

							case Wire::ComponentRegistry::PropertyType::Enum:
							{
								auto& enumData = Wire::ComponentRegistry::EnumData();
								if (enumData.find(prop.enumName) != enumData.end())
								{
									UI::ComboProperty(prop.name, *(int32_t*)(&data[prop.offset]), enumData.at(prop.enumName));
								}
								break;
							}
						}
					}

					UI::EndProperties();
				}
				else if (registryInfo.name == "ScriptComponent")
				{
					Volt::ScriptComponent& scriptComp = registry.GetComponent<Volt::ScriptComponent>(entity);
					if (UI::BeginProperties("scriptComponent"))
					{
						for (const auto& script : scriptComp.scripts)
						{
							std::string name = Volt::ScriptRegistry::GetNameFromGUID(script);
							UI::Property("Name", name);
						}
						UI::EndProperties();
					}

				}
				else if (registryInfo.name == "MonoScriptComponent")
				{
					DrawMonoProperties(registry, registryInfo, entity);
				}
				UI::PopId();

				UI::TreeNodePop();
			}

			if (removeComp)
			{
				myCurrentScene->GetRegistry().RemoveComponent(registryInfo.guid, entity);
			}
		}
	}
	else
	{
		auto componentsInCommon = registry.GetPools();

		for (auto& entity : entities)
		{
			for (const auto& [guid, pool] : registry.GetPools())
			{
				if (!registry.HasComponent(guid, entity))
				{
					componentsInCommon.erase(guid);
					continue;
				}
			}
		}

		const auto& entity = firstEntity;

		for (const auto& [guid, pool] : componentsInCommon)
		{
			if (!registry.HasComponent(guid, entity))
			{
				continue;
			}

			const auto& registryInfo = Wire::ComponentRegistry::GetRegistryDataFromGUID(guid);
			if (registryInfo.name == "TagComponent" || registryInfo.name == "TransformComponent" || registryInfo.name == "RelationshipComponent" || registryInfo.name == "PrefabComponent" ||
				registryInfo.name == "EntityDataComponent"/* || registryInfo.name == "NavMeshComponent"*/)
			{
				continue;
			}

			bool removeComp = false;
			bool open = UI::TreeNodeFramed(registryInfo.name.c_str(), true, 2.f);
			float buttonSize = 21.f + GImGui->Style.FramePadding.y;
			float availRegion = ImGui::GetContentRegionAvail().x;

			if (!open)
			{
				UI::SameLine(availRegion - buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
			}
			else
			{
				UI::SameLine(availRegion + buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
			}
			std::string id = "-###Remove" + registryInfo.name;

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
				UI::PushId();
				if (registryInfo.name != "ScriptComponent" && UI::BeginProperties(registryInfo.name))
				{
					uint8_t* data = (uint8_t*)registry.GetComponentPtr(guid, entity);
					for (const auto& prop : registryInfo.properties)
					{
						if (!prop.visible)
						{
							continue;
						}

						switch (prop.type)
						{
							case Wire::ComponentRegistry::PropertyType::Bool: UI::Property(prop.name, *(bool*)(&data[prop.offset]), [&](bool& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(bool*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::String: UI::Property(prop.name, *(std::string*)(&data[prop.offset]), false, [&](std::string& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(std::string*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Int: UI::Property(prop.name, *(int32_t*)(&data[prop.offset]), [&](int32_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(int32_t*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::UInt: UI::Property(prop.name, *(uint32_t*)(&data[prop.offset]), [&](uint32_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(uint32_t*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::EntityId: UI::PropertyEntity(prop.name, myCurrentScene, *(Wire::EntityId*)(&data[prop.offset]), [&](Wire::EntityId& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(Wire::EntityId*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Short: UI::Property(prop.name, *(int16_t*)(&data[prop.offset]), [&](int16_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(int16_t*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::UShort: UI::Property(prop.name, *(uint16_t*)(&data[prop.offset]), [&](uint16_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(uint16_t*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Char: UI::Property(prop.name, *(int8_t*)(&data[prop.offset]), [&](int8_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(int8_t*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::UChar: UI::Property(prop.name, *(uint8_t*)(&data[prop.offset]), [&](uint8_t& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(uint8_t*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Float: UI::Property(prop.name, *(float*)(&data[prop.offset]), false, 0.f, 0.f, [&](float& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(float*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::Double: UI::Property(prop.name, *(double*)(&data[prop.offset]), [&](double& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(double*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Vector2: UI::Property(prop.name, *(gem::vec2*)(&data[prop.offset]), 0.f, 0.f, [&](gem::vec2& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec2*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::Vector3: UI::Property(prop.name, *(gem::vec3*)(&data[prop.offset]), 0.f, 0.f, [&](gem::vec3& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec3*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::Vector4: UI::Property(prop.name, *(gem::vec4*)(&data[prop.offset]), 0.f, 0.f, [&](gem::vec4& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec4*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Quaternion: UI::Property(prop.name, *(gem::vec4*)(&data[prop.offset]), 0.f, 0.f, [&](gem::vec4& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec4*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Color3: UI::PropertyColor(prop.name, *(gem::vec3*)(&data[prop.offset]), [&](gem::vec3& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec3*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::Color4: UI::PropertyColor(prop.name, *(gem::vec4*)(&data[prop.offset]), [&](gem::vec4& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(gem::vec4*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::AssetHandle:
							{
								Volt::AssetType assetType = Volt::AssetType::None;

								if (prop.specialType.has_value())
								{
									assetType = std::any_cast<Volt::AssetType>(prop.specialType);
								}

								EditorUtils::Property(prop.name, *(Volt::AssetHandle*)(&data[prop.offset]), assetType, [&](Volt::AssetHandle& val)
									{
										for (auto& ent : entities)
										{
											uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
											*(Volt::AssetHandle*)&entData[prop.offset] = val;
										}
									});
								break;
							}
							case Wire::ComponentRegistry::PropertyType::Folder: UI::PropertyDirectory(prop.name, *(std::filesystem::path*)(&data[prop.offset]), [&](std::filesystem::path& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(std::filesystem::path*)&entData[prop.offset] = val;
									}
								}); break;
							case Wire::ComponentRegistry::PropertyType::Path: UI::Property(prop.name, *(std::filesystem::path*)(&data[prop.offset]), [&](std::filesystem::path& val)
								{
									for (auto& ent : entities)
									{
										uint8_t* entData = (uint8_t*)registry.GetComponentPtr(guid, ent);
										*(std::filesystem::path*)&entData[prop.offset] = val;
									}
								}); break;

							case Wire::ComponentRegistry::PropertyType::Enum:
							{
								auto& enumData = Wire::ComponentRegistry::EnumData();
								if (enumData.find(prop.enumName) != enumData.end())
								{
									UI::ComboProperty(prop.name, *(int32_t*)(&data[prop.offset]), enumData.at(prop.enumName));
								}
								break;
							}
						}
					}

					UI::EndProperties();
				}
				UI::PopId();

				UI::TreeNodePop();
			}

			if (removeComp)
			{
				for (auto& ent : entities)
				{
					myCurrentScene->GetRegistry().RemoveComponent(registryInfo.guid, ent);
				}
			}
		}
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
			myHasComponentSearchQuery = false;
			myComponentSearchQuery = "";
			mySearchedComponentNames.clear();
			UI::OpenPopup("AddComponent");
		}

		ImGui::TableNextColumn();

		if (ImGui::Button("Add Script", { width, buttonHeight }))
		{
			myHasScriptSearchQuery = false;
			myScriptSearchQuery = "";
			mySearchedScriptNames.clear();
			UI::OpenPopup("AddScript");
		}

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	AddComponentPopup();
	AddScriptPopup();
}

void PropertiesPanel::AddComponentPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (UI::BeginPopup("AddComponent", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		const std::vector<std::string> skippedComponents = {/* "NavMeshComponent",*/ "TagComponent", "PrefabComponent", "TransformComponent", "RelationshipComponent" };
		std::vector<std::string> componentNames;

		const auto& componentInfo = Wire::ComponentRegistry::ComponentGUIDs();
		for (const auto& [name, info] : componentInfo)
		{
			componentNames.emplace_back(name);
		}

		std::sort(componentNames.begin(), componentNames.end());

		// Seach bar
		{
			if (EditorUtils::SearchBar(myComponentSearchQuery, myHasComponentSearchQuery))
			{
				mySearchedComponentNames.clear();

				for (const auto& name : componentNames)
				{
					if (std::find(skippedComponents.begin(), skippedComponents.end(), name) != skippedComponents.end())
					{
						continue;
					}

					if (Utils::ToLower(name).find(Utils::ToLower(myComponentSearchQuery)) != std::string::npos)
					{
						mySearchedComponentNames.emplace_back(name);
					}
				}
			}
		}

		{
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			if (myHasComponentSearchQuery)
			{
				componentNames = mySearchedComponentNames;
			}

			for (const auto& name : componentNames)
			{
				if (std::find(skippedComponents.begin(), skippedComponents.end(), name) != skippedComponents.end())
				{
					continue;
				}

				const auto& info = Wire::ComponentRegistry::GetRegistryDataFromName(name);

				if (!myCurrentScene->GetRegistry().HasComponent(info.guid, SelectionManager::GetSelectedEntities().front()) && ImGui::MenuItem(name.c_str()))
				{
					for (auto& ent : SelectionManager::GetSelectedEntities())
					{
						if (!myCurrentScene->GetRegistry().HasComponent(info.guid, ent))
						{
							myCurrentScene->GetRegistry().AddComponent(info.guid, ent);
						}
					}

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndChild();
		}

		UI::EndPopup();
	}
}

void PropertiesPanel::AddScriptPopup()
{
	ImGui::SetNextWindowSize({ 250.f, 500.f });
	if (UI::BeginPopup("AddScript"))
	{
		const auto& scriptInfo = Volt::ScriptRegistry::GetRegistry();

		std::vector<std::string> scriptNames;

		for (const auto& [GUID, info] : scriptInfo)
		{
			scriptNames.emplace_back(info.name);
		}

		std::sort(scriptNames.begin(), scriptNames.end());

		// Seach bar
		{
			if (EditorUtils::SearchBar(myScriptSearchQuery, myHasScriptSearchQuery))
			{
				mySearchedScriptNames.clear();

				myHasScriptSearchQuery = true;
				for (const auto& name : scriptNames)
				{
					if (Utils::ToLower(name).find(Utils::ToLower(myScriptSearchQuery)) != std::string::npos)
					{
						mySearchedScriptNames.emplace_back(name);
					}
				}
			}
		}

		{
			ImGui::BeginChild("scrolling", ImGui::GetContentRegionAvail());

			if (myHasScriptSearchQuery)
			{
				scriptNames = mySearchedScriptNames;
			}

			for (const auto& name : scriptNames)
			{
				if (ImGui::MenuItem(name.c_str()))
				{
					const auto& guid = Volt::ScriptRegistry::GetGUIDFromName(name);

					for (auto& ent : SelectionManager::GetSelectedEntities())
					{
						if (!myCurrentScene->GetRegistry().HasComponent<Volt::ScriptComponent>(ent))
						{
							myCurrentScene->GetRegistry().AddComponent<Volt::ScriptComponent>(ent);
						}

						auto& comp = myCurrentScene->GetRegistry().GetComponent<Volt::ScriptComponent>(ent);
						if (std::find(comp.scripts.begin(), comp.scripts.end(), guid) != comp.scripts.end())
						{
							continue;
						}

						comp.scripts.emplace_back(guid);
					}
				}
			}

			ImGui::EndChild();
		}

		UI::EndPopup();
	}
}

void PropertiesPanel::DrawMonoProperties(Wire::Registry& registry, const Wire::ComponentRegistry::RegistrationInfo& registryInfo, Wire::EntityId entity)
{
	Volt::MonoScriptComponent& scriptComp = registry.GetComponent<Volt::MonoScriptComponent>(entity);
	Ref<Volt::MonoScriptInstance> scriptInstance = Volt::MonoScriptEngine::GetInstanceFromEntityId(entity);
	if (UI::BeginProperties(registryInfo.name))
	{
		UI::Property("Script", scriptComp.script);

		if (myCurrentScene->IsPlaying() && scriptInstance)
		{
			for (const auto& [name, field] : scriptInstance->GetClass()->GetFields())
			{
				switch (field.type)
				{
					case Wire::ComponentRegistry::PropertyType::Bool:
					{
						bool value = scriptInstance->GetField<bool>(name);
						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}
					//case Wire::ComponentRegistry::PropertyType::String:
					//{
					//	if (UI::Property(name, *(std::string*)(scriptInstance->GetField(name))))
					//	{
					//		scriptInstance->SetField(name, scriptInstance->GetField(name));
					//	}
					//	break;
					//}

					case Wire::ComponentRegistry::PropertyType::Int:
					{
						int32_t value = scriptInstance->GetField<int32_t>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::UInt:
					{
						uint32_t value = scriptInstance->GetField<uint32_t>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::Short:
					{
						int16_t value = scriptInstance->GetField<int16_t>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::UShort:
					{
						uint16_t value = scriptInstance->GetField<uint16_t>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::Char:
					{
						int8_t value = scriptInstance->GetField<int8_t>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::UChar:
					{
						uint8_t value = scriptInstance->GetField<uint8_t>(name);
						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::Float:
					{
						float value = scriptInstance->GetField<float>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::Double:
					{
						double value = scriptInstance->GetField<double>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::Vector2:
					{
						gem::vec2 value = scriptInstance->GetField<gem::vec2>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::Vector3:
					{
						gem::vec3 value = scriptInstance->GetField<gem::vec3>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}

					case Wire::ComponentRegistry::PropertyType::Vector4:
					{
						gem::vec4 value = scriptInstance->GetField<gem::vec4>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::Quaternion:
					{
						gem::vec4 value = scriptInstance->GetField<gem::vec4>(name);

						if (UI::Property(name, value))
						{
							scriptInstance->SetField(name, &value);
						}

						break;
					}

					case Wire::ComponentRegistry::PropertyType::EntityId:
					{
						Wire::EntityId value = scriptInstance->GetField<Wire::EntityId>(name);

						if (UI::PropertyEntity(name, myCurrentScene, value))
						{
							scriptInstance->SetField(name, &value);
						}
						break;
					}
				}
			}
		}
		else
		{
			if (Volt::MonoScriptEngine::EntityClassExists(scriptComp.script))
			{
				const auto& classFields = Volt::MonoScriptEngine::GetScriptClass(scriptComp.script)->GetFields();
				auto& entityFields = Volt::MonoScriptEngine::GetScriptFieldMap(entity);

				for (const auto& [name, field] : classFields)
				{
					if (entityFields.contains(name))
					{
						auto& entField = entityFields.at(name);

						switch (field.type)
						{
							case Wire::ComponentRegistry::PropertyType::Bool: UI::Property(name, *(bool*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::String: UI::Property(name, *(std::string*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Int: UI::Property(name, *(int32_t*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::UInt: UI::Property(name, *(uint32_t*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Short: UI::Property(name, *(int16_t*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::UShort: UI::Property(name, *(uint16_t*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Char: UI::Property(name, *(int8_t*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::UChar: UI::Property(name, *(uint8_t*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Float: UI::Property(name, *(float*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::Double: UI::Property(name, *(double*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Vector2: UI::Property(name, *(gem::vec2*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::Vector3: UI::Property(name, *(gem::vec3*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::Vector4: UI::Property(name, *(gem::vec4*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::Quaternion: UI::Property(name, *(gem::vec4*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::EntityId: UI::PropertyEntity(name, myCurrentScene, *(Wire::EntityId*)(&entField.data[0])); break;

							case Wire::ComponentRegistry::PropertyType::Color3: UI::PropertyColor(name, *(gem::vec3*)(&entField.data[0])); break;
							case Wire::ComponentRegistry::PropertyType::Color4: UI::PropertyColor(name, *(gem::vec4*)(&entField.data[0])); break;
						}
					}
					else
					{
						switch (field.type)
						{
							case Wire::ComponentRegistry::PropertyType::Bool:
							{
								bool value = false;
								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}
							//case Wire::ComponentRegistry::PropertyType::String:
							//{
							//	if (UI::Property(name, *(std::string*)(scriptInstance->GetField(name))))
							//	{
							//		scriptInstance->SetField(name, scriptInstance->GetField(name));
							//	}
							//	break;
							//}

							case Wire::ComponentRegistry::PropertyType::Int:
							{
								int32_t value = 0;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::UInt:
							{
								uint32_t value = 0u;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::Short:
							{
								int16_t value = 0;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::UShort:
							{
								uint16_t value = 0u;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::Char:
							{
								int8_t value = 0;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::UChar:
							{
								uint8_t value = 0u;
								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::Float:
							{
								float value = 0.f;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::Double:
							{
								double value = 0.0;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::Vector2:
							{
								gem::vec2 value = 0.f;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::Vector3:
							{
								gem::vec3 value = 0.f;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}

							case Wire::ComponentRegistry::PropertyType::Vector4:
							{
								gem::vec4 value = 0.f;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::Quaternion:
							{
								gem::vec4 value = 0.f;

								if (UI::Property(name, value))
								{
									entityFields[name].SetValue(value);
								}

								break;
							}

							case Wire::ComponentRegistry::PropertyType::EntityId:
							{
								Wire::EntityId value = 0;

								if (UI::PropertyEntity(name, myCurrentScene, value))
								{
									entityFields[name].SetValue(value);
								}
								break;
							}
						}
					}
				}
			}
		}
		UI::EndProperties();
	}
}
