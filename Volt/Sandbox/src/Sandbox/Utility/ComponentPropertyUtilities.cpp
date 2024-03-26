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
void RegisterMonoPropertyType(std::unordered_map<std::type_index, std::function<bool(const std::string&, Ref<Volt::MonoScriptInstance>)>>& outFunctionMap)
{
	outFunctionMap[std::type_index{ typeid(T) }] = [](const std::string& name, Ref<Volt::MonoScriptInstance> scriptInstance)
	{
		T value = scriptInstance->GetField<T>(name);
		const bool changed = UI::Property(name, value);

		if (changed)
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

		return changed;
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
	RegisterMonoPropertyType<glm::quat>(s_monoPropertyFunctions);

	RegisterMonoPropertyType<std::string>(s_monoPropertyFunctions);
}

void ComponentPropertyUtility::DrawComponents(Weak<Volt::Scene> scene, Volt::Entity entity)
{
	if (!s_initialized)
	{
		Initialize();
		s_initialized = true;
	}

	auto scenePtr = scene;
	auto& registry = scenePtr->GetRegistry();

	for (auto&& curr : registry.storage())
	{
		if (auto& storage = curr.second; storage.contains(entity))
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
						break;
					}

					bool removeComp = false;
					bool open = UI::CollapsingHeader(compTypeDesc->GetLabel(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
					float buttonSize = 21.f + GImGui->Style.FramePadding.y * 0.5f;
					float availRegion = ImGui::GetContentRegionAvail().x;

					UI::SameLine(availRegion - buttonSize * 0.5f + 1.f);

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

					DrawComponent(scene, entity, compTypeDesc, storage.get(entity), 0, open, false);

					if (removeComp)
					{
						Volt::ComponentRegistry::Helpers::RemoveComponentWithGUID(compTypeDesc->GetGUID(), scene->GetRegistry(), entity);
					}

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

void ComponentPropertyUtility::DrawComponent(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset, bool isOpen, bool isSubSection)
{
	if (isSubSection)
	{
		if (componentType->IsHidden())
		{
			return;
		}
	}

	if (!isOpen)
	{
		return;
	}

	if (UI::BeginProperties(std::string(componentType->GetLabel())))
	{
		for (const auto& member : componentType->GetMembers())
		{
			if ((member.flags & Volt::ComponentMemberFlag::DoNotShow) != Volt::ComponentMemberFlag::None)
			{
				continue;
			}

			if (member.typeDesc)
			{
				switch (member.typeDesc->GetValueType())
				{
					case Volt::ValueType::Component:
					{
						const Volt::IComponentTypeDesc* compDesc = reinterpret_cast<const Volt::IComponentTypeDesc*>(member.typeDesc);
						bool open = UI::CollapsingHeader(compDesc->GetLabel());
						DrawComponent(scene, entity, compDesc, data, offset + member.offset, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(member.typeDesc);
						DrawComponentEnum(scene, entity, member, enumDesc, data, offset + member.offset);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* arrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(member.typeDesc);
						DrawComponentArray(scene, entity, member, arrayDesc, data, offset + member.offset);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMember(scene, entity, member, data, offset);
			}
		}

		UI::EndProperties();
	}
}

void ComponentPropertyUtility::DrawComponentDefaultMember(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);

	if (member.assetType != Volt::AssetType::None)
	{
		if (EditorUtils::Property(std::string(member.label), *reinterpret_cast<Volt::AssetHandle*>(&bytePtr[offset + member.offset]), member.assetType))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		}
		return;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec3*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		}
		return;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec4*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		}
		return;
	}

	// Special case for entities
	if (member.typeIndex == std::type_index{ typeid(Volt::EntityID) })
	{
		if (UI::PropertyEntity(std::string(member.label), scene, *reinterpret_cast<Volt::EntityID*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		}
		return;
	}

	if (!s_propertyFunctions.contains(member.typeIndex))
	{
		return;
	}

	if (s_propertyFunctions.at(member.typeIndex)(member.label, data, offset + member.offset))
	{
		AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
	}
}

void ComponentPropertyUtility::DrawComponentDefaultMemberArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& arrayMember, void* elementData, const size_t index, const std::type_index& typeIndex)
{
	const std::string label = std::format("Element {0}", index);

	if (arrayMember.assetType != Volt::AssetType::None)
	{
		if (EditorUtils::Property(label, *reinterpret_cast<Volt::AssetHandle*>(elementData), arrayMember.assetType))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		}
		return;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(label, *reinterpret_cast<glm::vec3*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		}
		return;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(label, *reinterpret_cast<glm::vec4*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		}
		return;
	}

	// Special case for entities
	if (arrayMember.typeIndex == std::type_index{ typeid(Volt::EntityID) })
	{
		if (UI::PropertyEntity(label, scene, *reinterpret_cast<Volt::EntityID*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		}
		return;
	}

	if (!s_propertyFunctions.contains(typeIndex))
	{
		return;
	}

	if (s_propertyFunctions.at(typeIndex)(label, elementData, 0))
	{
		AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
		EditorUtils::MarkEntityAsEdited(entity);
	}
}

void ComponentPropertyUtility::DrawComponentEnum(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset)
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
		AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		EditorUtils::MarkEntityAsEdited(entity);
	}
}

void ComponentPropertyUtility::DrawComponentArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IArrayTypeDesc* arrayDesc, void* data, const size_t offset)
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
						DrawComponent(scene, entity, compDesc, elementData, 0, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(elementTypeDesc);
						DrawComponentEnum(scene, entity, member, enumDesc, elementData, 0);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* elementArrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(elementTypeDesc);
						DrawComponentArray(scene, entity, member, elementArrayDesc, elementData, 0);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMemberArray(scene, entity, member, elementData, i, elementTypeIndex);
			}
		}

		if (ImGui::Button((std::string("Add##add_") + std::string(member.label)).c_str()))
		{
			arrayDesc->EmplaceBack(arrayPtr, nullptr);
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);

			EditorUtils::MarkEntityAsEdited(entity);
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
	const bool open = UI::CollapsingHeader(scriptClassName + " Script", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);

	float buttonSize = 21.f + GImGui->Style.FramePadding.y * 0.5f;
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

	UI::SameLine(availRegion - buttonSize * 0.5f + 1.f);

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
			auto scenePtr = scene;

			scenePtr->ShutdownEngineScripts();
			scenePtr->InitializeEngineScripts();
		}
	}
}

void ComponentPropertyUtility::DrawMonoMembers(Weak<Volt::Scene> scene, const Volt::MonoScriptEntry& scriptEntry, Volt::Entity entity)
{
	Ref<Volt::MonoScriptInstance> scriptInstance = Volt::MonoScriptEngine::GetInstanceFromId(scriptEntry.id);

	if (!UI::BeginProperties("MonoScripts" + std::string("##") + scriptEntry.name))
	{
		return;
	}

	if (scene->IsPlaying() && scriptInstance)
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
			else if (field.type.IsEntity())
			{
				Volt::EntityID value = scriptInstance->GetField<Volt::EntityID>(name);
				if (UI::PropertyEntity(displayName, scene, value))
				{
					scriptInstance->SetField(name, &value);
					AddLocalChangeToEntity(entity, scriptEntry.name, name);
				}
			}
			else if (field.type.IsCustomMonoType())
			{
				Volt::EntityID value = scriptInstance->GetCustomMonoTypeField(name);
				if (UI::PropertyEntityCustomMonoType(displayName, scene, value, field.type))
				{

				}
			}
			else if ((field.type.typeFlags & Volt::MonoTypeFlags::Color) != Volt::MonoTypeFlags::None)
			{
				glm::vec4 value = scriptInstance->GetField<glm::vec4>(name);
				if (UI::PropertyColor(displayName, value))
				{
					scriptInstance->SetField(name, &value);
					AddLocalChangeToEntity(entity, scriptEntry.name, name);
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
						AddLocalChangeToEntity(entity, scriptEntry.name, name);
					}
				}

				continue;
			}

			if (s_monoPropertyFunctions.contains(field.type.typeIndex))
			{
				if (s_monoPropertyFunctions.at(field.type.typeIndex)(displayName, scriptInstance))
				{
					AddLocalChangeToEntity(entity, scriptEntry.name, name);
				}
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

		auto& entityFields = scene->GetScriptFieldCache().GetCache()[scriptEntry.id];

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

			// #TODO_Ivar: We probably should not have to check this here
			if (!currentField->data.IsValid())
			{
				const auto& defaultValueData = defaultFieldValueMap.at(name)->data;
				currentField->SetValue(defaultValueData.As<const void>(), defaultValueData.GetSize());
			}

			bool fontChanged = false;

			if (currentField->data.IsValid() && !defaultFieldValueMap.at(name)->data.IsValid())
			{
				fontChanged = true;
				UI::PushFont(FontType::Bold_17);
			}
			else if (!currentField->field.type.equalFunc(currentField->data.As<void>(), defaultFieldValueMap.at(name)->data.As<void>()))
			{
				fontChanged = true;
				UI::PushFont(FontType::Bold_17);
			}

			bool fieldChanged = false;

			if (field.type.IsString())
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

					fieldChanged = true;
				}
			}
			else if (field.type.IsEntity())
			{
				fieldChanged = UI::PropertyEntity(displayName, scene, *currentField->data.As<Volt::EntityID>());
			}
			else if (field.type.IsAsset())
			{
				fieldChanged = EditorUtils::Property(displayName, *currentField->data.As<Volt::AssetHandle>(), field.type.assetType);
			}
			else if ((field.type.typeFlags & Volt::MonoTypeFlags::Color) != Volt::MonoTypeFlags::None)
			{
				fieldChanged = UI::PropertyColor(displayName, *currentField->data.As<glm::vec4>());
			}
			else if (field.type.IsCustomMonoType())
			{
				fieldChanged = UI::PropertyEntityCustomMonoType(displayName, scene, *currentField->data.As<Volt::EntityID>(), field.type);
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
				AddLocalChangeToEntity(entity, scriptEntry.name, name);
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

void ComponentPropertyUtility::AddLocalChangeToEntity(Volt::Entity entity, const VoltGUID& componentGuid, std::string_view memberName)
{
	if (!entity.HasComponent<Volt::PrefabComponent>())
	{
		return;
	}

	auto& prefabComp = entity.GetComponent<Volt::PrefabComponent>();
	const std::string strMemberName = std::string(memberName);

	auto it = std::find_if(prefabComp.componentLocalChanges.begin(), prefabComp.componentLocalChanges.end(), [&](const auto& lhs)
	{
		return lhs.memberName == strMemberName && lhs.componentGUID == componentGuid;
	});

	if (it != prefabComp.componentLocalChanges.end())
	{
		return;
	}

	auto& newChange = prefabComp.componentLocalChanges.emplace_back();
	newChange.componentGUID = componentGuid;
	newChange.memberName = strMemberName;

	EditorUtils::MarkEntityAsEdited(entity);
}

void ComponentPropertyUtility::AddLocalChangeToEntity(Volt::Entity entity, const std::string& scriptName, std::string_view memberName)
{
	if (!entity.HasComponent<Volt::PrefabComponent>())
	{
		return;
	}

	auto& prefabComp = entity.GetComponent<Volt::PrefabComponent>();
	const std::string strMemberName = std::string(memberName);

	auto it = std::find_if(prefabComp.scriptLocalChanges.begin(), prefabComp.scriptLocalChanges.end(), [&](const auto& lhs)
	{
		return lhs.memberName == memberName && lhs.scriptName == scriptName;
	});

	if (it != prefabComp.scriptLocalChanges.end())
	{
		return;
	}

	auto& newChange = prefabComp.scriptLocalChanges.emplace_back();
	newChange.scriptName = scriptName;
	newChange.memberName = memberName;

	EditorUtils::MarkEntityAsEdited(entity);
}
