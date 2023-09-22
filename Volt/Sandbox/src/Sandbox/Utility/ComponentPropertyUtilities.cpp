#include "sbpch.h"
#include "ComponentPropertyUtilities.h"

#include "Sandbox/Sandbox.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Sandbox/UserSettingsManager.h"

#include <Volt/Scene/Reflection/ComponentReflection.h>
#include <Volt/Scene/Reflection/ComponentRegistry.h>

#include <Volt/Components/LightComponents.h>

#include <Volt/Scripting/Mono/MonoScriptUtils.h>
#include <Volt/Scripting/Mono/MonoScriptInstance.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>
#include <Volt/Scripting/Mono/MonoEnum.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/PremadeCommands.h>

#include <glm/glm.hpp>

template<typename T>
void RegisterPropertyType(std::unordered_map<std::type_index, std::function<bool(std::string_view, void*, const size_t)>>& outFunctionMap)
{
	outFunctionMap[std::type_index{ typeid(T) }] = [](std::string_view label, void* data, const size_t offset) -> bool
	{
		uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
		return UI::Property(std::string(label), *reinterpret_cast<T*>(&bytePtr[offset]));
	};
}

template<typename T>
void RegisterMonoPropertyType(std::unordered_map<std::type_index, std::function<void(const std::string&, Ref<Volt::MonoScriptInstance>)>>& outFunctionMap)
{
	outFunctionMap[std::type_index{ typeid(T) }] = [](const std::string& name, Ref<Volt::MonoScriptInstance> scriptInstance)
	{
		T value = scriptInstance->GetField<T>(name);
		if (UI::Property(name, value))
		{
			if constexpr (std::is_same<T, std::string>::value)
			{
				scriptInstance->SetField(name, value);
			}
			else
			{
				scriptInstance->SetField(name, &value);
			}
		}
	};
}

void ComponentPropertyUtility::Initialize()
{
	// Components
	RegisterPropertyType<int8_t>(s_propertyFunctions);
	RegisterPropertyType<uint8_t>(s_propertyFunctions);
	RegisterPropertyType<int16_t>(s_propertyFunctions);
	RegisterPropertyType<uint16_t>(s_propertyFunctions);
	RegisterPropertyType<int32_t>(s_propertyFunctions);
	RegisterPropertyType<uint32_t>(s_propertyFunctions);

	RegisterPropertyType<float>(s_propertyFunctions);
	RegisterPropertyType<double>(s_propertyFunctions);
	RegisterPropertyType<bool>(s_propertyFunctions);

	RegisterPropertyType<glm::vec2>(s_propertyFunctions);
	RegisterPropertyType<glm::vec3>(s_propertyFunctions);
	RegisterPropertyType<glm::vec4>(s_propertyFunctions);

	RegisterPropertyType<glm::uvec2>(s_propertyFunctions);
	RegisterPropertyType<glm::uvec3>(s_propertyFunctions);
	RegisterPropertyType<glm::uvec4>(s_propertyFunctions);

	RegisterPropertyType<glm::ivec2>(s_propertyFunctions);
	RegisterPropertyType<glm::ivec3>(s_propertyFunctions);
	RegisterPropertyType<glm::ivec4>(s_propertyFunctions);

	RegisterPropertyType<std::string>(s_propertyFunctions);
	RegisterPropertyType<std::filesystem::path>(s_propertyFunctions);

	// Mono scipts
	RegisterMonoPropertyType<int8_t>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<uint8_t>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<int16_t>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<uint16_t>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<int32_t>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<uint32_t>(s_monoPropertyFunctions);

	RegisterMonoPropertyType<float>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<double>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<bool>(s_monoPropertyFunctions);

	RegisterMonoPropertyType<glm::vec2>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<glm::vec3>(s_monoPropertyFunctions);
	RegisterMonoPropertyType<glm::vec4>(s_monoPropertyFunctions);

	RegisterMonoPropertyType<std::string>(s_monoPropertyFunctions);
}

void ComponentPropertyUtility::DrawComponents(Weak<Volt::Scene> scene, Volt::Entity entity)
{
	if (!s_initialized)
	{
		Initialize();
		s_initialized = true;
	}

	auto scenePtr = scene.lock();
	auto& registry = scenePtr->GetRegistry();

	for (auto&& curr : registry.storage())
	{
		if (auto& storage = curr.second; storage.contains(entity.GetID()))
		{
			std::string_view typeName = storage.type().name();
			const Volt::ICommonTypeDesc* typeDesc = Volt::ComponentRegistry::GetTypeDescFromName(typeName);
			if (!typeDesc)
			{
				continue;
			}

			switch (typeDesc->GetValueType())
			{
				case Volt::ValueType::Component:
				{
					const Volt::IComponentTypeDesc* compTypeDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(typeDesc);

					if (compTypeDesc->IsHidden())
					{
						return;
					}

					bool removeComp = false;
					bool open = UI::CollapsingHeader(compTypeDesc->GetLabel(), ImGuiTreeNodeFlags_DefaultOpen);
					float buttonSize = 22.f + GImGui->Style.FramePadding.y * 0.5f;
					float availRegion = ImGui::GetContentRegionAvail().x;

					if (!open)
					{
						UI::SameLine(availRegion - buttonSize * 0.5f);
					}
					else
					{
						UI::SameLine(availRegion + buttonSize * 0.5f);
					}

					std::string id = "-###Remove" + std::string(compTypeDesc->GetLabel());

					{
						UI::ScopedStyleFloat round{ ImGuiStyleVar_FrameRounding, 0.f };
						UI::ScopedStyleFloat2 pad{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
						UI::ScopedButtonColor buttonColor{ EditorTheme::Buttons::RemoveButton };

						if (ImGui::Button(id.c_str(), ImVec2{ buttonSize, buttonSize }))
						{
							removeComp = true;
						}
					}

					DrawComponent(scene, compTypeDesc, storage.get(entity.GetID()), 0, open, false);
					break;
				}
			}
		}
	}
}

void ComponentPropertyUtility::DrawMonoScripts(Weak<Volt::Scene> scene, Volt::Entity entity)
{
	if (!entity.HasComponent<Volt::MonoScriptComponent>())
	{
		return;
	}

	if (!s_initialized)
	{
		Initialize();
		s_initialized = true;
	}

	auto& monoScriptComp = entity.GetComponent<Volt::MonoScriptComponent>();
	for (size_t i = 0; i < monoScriptComp.scriptIds.size(); i++)
	{
		Volt::MonoScriptEntry entry{ monoScriptComp.scriptNames.at(i), monoScriptComp.scriptIds.at(i) };
		DrawMonoScript(scene, entry, entity);
	}
}

void ComponentPropertyUtility::DrawComponent(Weak<Volt::Scene> scene, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset, bool isOpen, bool isSubSection)
{
	if (isSubSection)
	{
		if (componentType->IsHidden())
		{
			return;
		}
	}

	if (isOpen && UI::BeginProperties(std::string(componentType->GetLabel())))
	{
		for (const auto& member : componentType->GetMembers())
		{
			if (member.typeDesc)
			{
				switch (member.typeDesc->GetValueType())
				{
					case Volt::ValueType::Component:
					{
						const Volt::IComponentTypeDesc* compDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(member.typeDesc);
						bool open = UI::CollapsingHeader(compDesc->GetLabel());
						DrawComponent(scene, compDesc, data, offset + member.offset, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(member.typeDesc);
						DrawComponentEnum(scene, member, enumDesc, data, offset + member.offset);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* arrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(member.typeDesc);
						DrawComponentArray(scene, member, arrayDesc, data, offset + member.offset);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMember(scene, member, data, offset);
			}
		}

		UI::EndProperties();
	}
}

void ComponentPropertyUtility::DrawComponentDefaultMember(Weak<Volt::Scene> scene, const Volt::ComponentMember& member, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);

	if (member.assetType != Volt::AssetType::None)
	{
		EditorUtils::Property(std::string(member.label), *reinterpret_cast<Volt::AssetHandle*>(&bytePtr[offset + member.offset]), member.assetType);
		return;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec3*>(&bytePtr[offset + member.offset]));
		return;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec4*>(&bytePtr[offset + member.offset]));
		return;
	}

	// Special case for entities
	if (member.typeIndex == std::type_index{ typeid(entt::entity) })
	{
		UI::PropertyEntity(std::string(member.label), scene.lock(), *reinterpret_cast<entt::entity*>(&bytePtr[offset + member.offset]));
		return;
	}

	if (!s_propertyFunctions.contains(member.typeIndex))
	{
		return;
	}

	s_propertyFunctions.at(member.typeIndex)(member.label, data, offset + member.offset);
}

void ComponentPropertyUtility::DrawComponentDefaultMemberArray(Weak<Volt::Scene> scene, const Volt::ComponentMember& arrayMember, void* elementData, const size_t index, const std::type_index& typeIndex)
{
	const std::string label = std::format("Element {0}", index);

	if (arrayMember.assetType != Volt::AssetType::None)
	{
		EditorUtils::Property(label, *reinterpret_cast<Volt::AssetHandle*>(elementData), arrayMember.assetType);
		return;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		UI::PropertyColor(label, *reinterpret_cast<glm::vec3*>(elementData));
		return;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		UI::PropertyColor(label, *reinterpret_cast<glm::vec4*>(elementData));
		return;
	}

	// Special case for entities
	if (arrayMember.typeIndex == std::type_index{ typeid(entt::entity) })
	{
		UI::PropertyEntity(label, scene.lock(), *reinterpret_cast<entt::entity*>(elementData));
		return;
	}

	if (!s_propertyFunctions.contains(typeIndex))
	{
		return;
	}

	s_propertyFunctions.at(typeIndex)(label, elementData, 0);
}

void ComponentPropertyUtility::DrawComponentEnum(Weak<Volt::Scene> scene, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
	const auto& constants = enumType->GetConstants();

	int32_t& currentValue = *reinterpret_cast<int32_t*>(&bytePtr[offset + member.offset]);
	int32_t currentIndex = 0;

	std::unordered_map<int32_t, int32_t> indexToValueMap;
	std::vector<std::string> constantNames;

	for (uint32_t index = 0; const auto & constant : constants)
	{
		const std::string name = std::string(constant.label);
		constantNames.emplace_back(name);
		indexToValueMap[index] = constant.value;

		if (constant.value == currentValue)
		{
			currentIndex = index;
		}

		index++;
	}

	if (UI::ComboProperty(std::string(member.label), currentValue, constantNames))
	{
		currentValue = indexToValueMap.at(currentValue);
	}
}

void ComponentPropertyUtility::DrawComponentArray(Weak<Volt::Scene> scene, const Volt::ComponentMember& member, const Volt::IArrayTypeDesc* arrayDesc, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
	void* arrayPtr = &bytePtr[offset];

	const Volt::ICommonTypeDesc* elementTypeDesc = arrayDesc->GetElementTypeDesc();
	const bool isNonDefaultType = elementTypeDesc != nullptr;
	const auto& elementTypeIndex = arrayDesc->GetElementTypeIndex();

	ImGui::TableNextColumn();
	const bool arrayOpen = ImGui::TreeNodeEx(member.label.data(), ImGuiTreeNodeFlags_SpanFullWidth);

	if (arrayOpen)
	{
		const size_t size = arrayDesc->Size(arrayPtr);
		for (size_t i = 0; i < size; i++)
		{
			void* elementData = arrayDesc->At(arrayPtr, i);

			if (isNonDefaultType)
			{
				switch (arrayDesc->GetElementTypeDesc()->GetValueType())
				{
					case Volt::ValueType::Component:
					{
						const Volt::IComponentTypeDesc* compDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(elementTypeDesc);
						ImGui::Text("Element %d", i);
						ImGui::TableNextColumn();

						bool open = UI::CollapsingHeader(std::string(compDesc->GetLabel()) + std::format("##{0}", i));
						DrawComponent(scene, compDesc, elementData, 0, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(elementTypeDesc);
						DrawComponentEnum(scene, member, enumDesc, elementData, 0);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* elementArrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(elementTypeDesc);
						DrawComponentArray(scene, member, elementArrayDesc, elementData, 0);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMemberArray(scene, member, elementData, i, elementTypeIndex);
			}
		}

		if (ImGui::Button((std::string("Add##add_") + std::string(member.label)).c_str()))
		{
			arrayDesc->EmplaceBack(arrayPtr, nullptr);
		}

		ImGui::TreePop();
	}

	if (!arrayOpen)
	{
		ImGui::TableNextColumn();
	}
}

void ComponentPropertyUtility::DrawMonoScript(Weak<Volt::Scene> scene, const Volt::MonoScriptEntry& scriptEntry, Volt::Entity entity)
{
	std::string scriptClassName = Volt::MonoScriptUtils::GetClassName(scriptEntry.name);
	scriptClassName[0] = static_cast<char>(std::toupper(scriptClassName[0]));

	bool removeScript = false;
	const bool open = UI::CollapsingHeader(scriptClassName + " Script", ImGuiTreeNodeFlags_DefaultOpen);

	const float buttonSize = 22.f + GImGui->Style.FramePadding.y * 0.5f;
	const float availRegion = ImGui::GetContentRegionAvail().x;

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

	const std::string id = "-###Remove" + std::format("{0}", static_cast<uint64_t>(scriptEntry.id));

	{
		UI::ScopedStyleFloat round{ ImGuiStyleVar_FrameRounding, 0.f };
		UI::ScopedStyleFloat2 pad{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		UI::ScopedColor color{ ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.f } };
		UI::ScopedColor colorh{ ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.f } };
		UI::ScopedColor colora{ ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.f } };

		if (ImGui::Button(id.c_str(), ImVec2{ buttonSize, buttonSize }))
		{
			removeScript = true;
		}
	}

	if (open)
	{
		UI::PushID();

		DrawMonoMembers(scene, scriptEntry, entity);

		UI::PopID();
	}

	if (removeScript)
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

		if (Sandbox::Get().GetSceneState() == SceneState::Edit)
		{
			auto scenePtr = scene.lock();

			scenePtr->ShutdownEngineScripts();
			scenePtr->InitializeEngineScripts();
		}
	}
}

void ComponentPropertyUtility::DrawMonoMembers(Weak<Volt::Scene> scene, const Volt::MonoScriptEntry& scriptEntry, Volt::Entity entity)
{
	Ref<Volt::MonoScriptInstance> scriptInstance = Volt::MonoScriptEngine::GetInstanceFromId(scriptEntry.id);

	if (!UI::BeginProperties("MonoScripts"))
	{
		return;
	}

	if (scene.lock()->IsPlaying() && scriptInstance)
	{
		for (const auto& [name, field] : scriptInstance->GetClass()->GetFields())
		{
			std::string displayName = name;
			if (field.netData.replicatedCondition == Volt::eRepCondition::CONTINUOUS)
			{
				displayName = "[C] " + displayName;
			}
			else if (field.netData.replicatedCondition == Volt::eRepCondition::NOTIFY)
			{
				displayName = "[N] " + displayName;
			}

			if (field.type.IsAsset())
			{
				Volt::AssetHandle value = scriptInstance->GetField<Volt::AssetHandle>(name);

				if (EditorUtils::Property(displayName, value, field.type.assetType))
				{
					scriptInstance->SetField(name, &value);
				}

				continue;
			}
			else if ((field.type.typeFlags & Volt::MonoTypeFlags::Color) != Volt::MonoTypeFlags::None)
			{
				glm::vec4 value = scriptInstance->GetField<glm::vec4>(name);
				if (UI::PropertyColor(displayName, value))
				{
					scriptInstance->SetField(name, &value);
				}

				continue;
			}
			else if (field.type.IsEnum())
			{
				const auto& registeredEnums = Volt::MonoScriptEngine::GetRegisteredEnums();
				if (registeredEnums.contains(std::string(field.type.typeName)))
				{
					const auto enumData = registeredEnums.at(std::string(field.type.typeName));
					const auto& enumValues = enumData->GetValues();

					int32_t enumVal = scriptInstance->GetField<int32_t>(name);
					
					std::vector<std::string> valueNames{};
					for (const auto& [valueName, val] : enumValues)
					{
						valueNames.emplace_back(valueName);
					}

					if (UI::ComboProperty(displayName, enumVal, valueNames))
					{
						scriptInstance->SetField(name, &enumVal);
					}
				}

				continue;
			}

			if (s_monoPropertyFunctions.contains(field.type.typeIndex))
			{
				s_monoPropertyFunctions.at(field.type.typeIndex)(displayName, scriptInstance);
			}
		}
	}
	else
	{
		if (!Volt::MonoScriptEngine::EntityClassExists(scriptEntry.name))
		{
			UI::EndProperties();
			return;
		}

		const auto& classFields = Volt::MonoScriptEngine::GetScriptClass(scriptEntry.name)->GetFields();
		const auto& defaultFieldValueMap = Volt::MonoScriptEngine::GetDefaultScriptFieldMap(scriptEntry.name);
	
		auto& entityFields = scene.lock()->GetScriptFieldCache().GetCache()[scriptEntry.id];
	
		for (const auto& [name, field] : classFields)
		{
			if (!entityFields.contains(name))
			{
				Ref<Volt::MonoScriptFieldInstance> fieldInstance = CreateRef<Volt::MonoScriptFieldInstance>();
				fieldInstance->field = field;

				const auto& defaultValueData = defaultFieldValueMap.at(name)->data;

				fieldInstance->data.Allocate(defaultValueData.GetSize());
				fieldInstance->data.Copy(defaultValueData.As<void>(), defaultValueData.GetSize());

				entityFields[name] = fieldInstance;
			}

			std::string displayName = name;
			if (field.netData.replicatedCondition == Volt::eRepCondition::CONTINUOUS)
			{
				displayName = "[C] " + displayName;
			}
			else if (field.netData.replicatedCondition == Volt::eRepCondition::NOTIFY)
			{
				displayName = "[N] " + displayName;
			}

			auto& currentField = entityFields.at(name);
			bool fontChanged = false;

			if (currentField->data.IsValid() && !defaultFieldValueMap.at(name)->data.IsValid())
			{
				fontChanged = true;
				UI::PushFont(FontType::Bold_17);
			}
			else if (memcmp(currentField->data.As<void>(), defaultFieldValueMap.at(name)->data.As<void>(), currentField->data.GetSize()) != 0) // #TODO_Ivar: Reimplement to use custom function instead
			{
				fontChanged = true;
				UI::PushFont(FontType::Bold_17);
			}

			bool fieldChanged = false;

			if (field.type.IsEntity())
			{
				std::string str;

				if (currentField->data.IsValid())
				{
					str = std::string(currentField->data.As<const char>());
				}

				if (UI::Property(displayName, str))
				{
					currentField->SetValue(str, str.size());
					if (scriptInstance)
					{
						scriptInstance->SetField(name, str);
					}
				}
			}
			else if (field.type.IsAsset())
			{
				fieldChanged = EditorUtils::Property(displayName, *currentField->data.As<Volt::AssetHandle>(), field.type.assetType);
			}
			else if ((field.type.typeFlags & Volt::MonoTypeFlags::Color) != Volt::MonoTypeFlags::None)
			{
				fieldChanged = UI::PropertyColor(displayName, *currentField->data.As<glm::vec4>());
			}
			else if (field.type.IsEnum())
			{
				const auto& registeredEnums = Volt::MonoScriptEngine::GetRegisteredEnums();
				if (registeredEnums.contains(std::string(field.type.typeName)))
				{
					const auto enumData = registeredEnums.at(std::string(field.type.typeName));
					const auto& enumValues = enumData->GetValues();

					int32_t& enumVal = (int32_t&)*currentField->data.As<uint32_t>();

					std::vector<std::string> valueNames{};
					for (const auto& [valueName, val] : enumValues)
					{
						valueNames.emplace_back(valueName);
					}

					fieldChanged = UI::ComboProperty(displayName, enumVal, valueNames);
				}
			}
			else
			{
				if (s_propertyFunctions.contains(field.type.typeIndex))
				{
					fieldChanged = s_propertyFunctions.at(field.type.typeIndex)(displayName, currentField->data.As<void>(), 0);
				}
			}

			if (scriptInstance && fieldChanged)
			{
				scriptInstance->SetField(name, currentField->data.As<void>());
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

	UI::EndProperties();
}
