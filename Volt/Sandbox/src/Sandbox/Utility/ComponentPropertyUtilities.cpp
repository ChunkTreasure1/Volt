#include "sbpch.h"
#include "ComponentPropertyUtilities.h"

#include "Sandbox/Sandbox.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include "Sandbox/UserSettingsManager.h"

#include <Volt/Components/LightComponents.h>

#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/PremadeCommands.h>

#include <EntitySystem/ComponentRegistry.h>

#include <glm/glm.hpp>

template<typename T>
void RegisterPropertyType(std::unordered_map<TypeTraits::TypeIndex, std::function<bool(std::string_view, void*, const size_t)>>& outFunctionMap)
{
	outFunctionMap[TypeTraits::TypeIndex::FromType<T>()] = [](std::string_view label, void* data, const size_t offset) -> bool
	{
		uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data); 
		return UI::Property(std::string(label), *reinterpret_cast<T*>(&bytePtr[offset]));
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
			const Volt::ICommonTypeDesc* typeDesc = GetComponentRegistry().GetTypeDescFromName(typeName);
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
						EditorUtils::MarkEntityAsEdited(entity);
					}

					break;
				}
			}
		}
	}
}

bool ComponentPropertyUtility::DrawComponent(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset, bool isOpen, bool isSubSection)
{
	if (isSubSection)
	{
		if (componentType->IsHidden())
		{
			return false;
		}
	}

	if (!isOpen)
	{
		return false;
	}

	bool edited = false;

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
						edited |= DrawComponent(scene, entity, compDesc, data, offset + member.offset, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(member.typeDesc);
						edited |= DrawComponentEnum(scene, entity, member, enumDesc, data, offset + member.offset);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* arrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(member.typeDesc);
						edited |= DrawComponentArray(scene, entity, member, arrayDesc, data, offset + member.offset);
						break;
					}
				}
			}
			else
			{
				edited |= DrawComponentDefaultMember(scene, entity, member, data, offset);
			}
		}

		UI::EndProperties();
	}

	if (edited)
	{
		uint8_t* offsetPtr = ((uint8_t*)data) + offset;
		componentType->OnMemberChanged(offsetPtr, entity);

		EditorUtils::MarkEntityAsEdited(entity);
	}

	return edited;
}

bool ComponentPropertyUtility::DrawComponentDefaultMember(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data); 

	if (member.GetAssetType() != AssetTypes::None)
	{
		if (EditorUtils::Property(std::string(member.label), *reinterpret_cast<Volt::AssetHandle*>(&bytePtr[offset + member.offset]), member.GetAssetType()))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
			return true;
		}

		return false;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec3*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
			return true;
		}

		return false;
	}

	if ((member.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(std::string(member.label), *reinterpret_cast<glm::vec4*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
			return true;
		}

		return false;
	}

	// Special case for entities
	if (member.typeIndex == TypeTraits::TypeIndex::FromType<Volt::EntityID>())
	{
		if (UI::PropertyEntity(std::string(member.label), scene, *reinterpret_cast<Volt::EntityID*>(&bytePtr[offset + member.offset])))
		{
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
			return true;
		}

		return false;
	}

	if (!s_propertyFunctions.contains(member.typeIndex))
	{
		return false;
	}

	if (s_propertyFunctions.at(member.typeIndex)(member.label, data, offset + member.offset))
	{
		AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);
		return true;
	}

	return false;
}

bool ComponentPropertyUtility::DrawComponentDefaultMemberArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& arrayMember, void* elementData, const size_t index, const TypeTraits::TypeIndex& typeIndex, AssetType arrayAssetType)
{
	const std::string label = std::format("Element {0}", index);

	if (arrayMember.GetAssetType() != AssetTypes::None || arrayAssetType != AssetTypes::None)
	{
		if (EditorUtils::Property(label, *reinterpret_cast<Volt::AssetHandle*>(elementData), arrayMember.GetAssetType() != AssetTypes::None ? arrayMember.GetAssetType() : arrayAssetType))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		
			return true;
		}
		return false;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color3) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(label, *reinterpret_cast<glm::vec3*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		
			return true;
		}
		return false;
	}

	if ((arrayMember.flags & Volt::ComponentMemberFlag::Color4) != Volt::ComponentMemberFlag::None)
	{
		if (UI::PropertyColor(label, *reinterpret_cast<glm::vec4*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
			
			return true;
		}
		return false;
	}

	// Special case for entities
	if (arrayMember.typeIndex == TypeTraits::TypeIndex::FromType<Volt::EntityID>())
	{
		if (UI::PropertyEntity(label, scene, *reinterpret_cast<Volt::EntityID*>(elementData)))
		{
			AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
			EditorUtils::MarkEntityAsEdited(entity);
		
			return true;
		}
		return false;
	}

	if (!s_propertyFunctions.contains(typeIndex))
	{
		return false;
	}

	if (s_propertyFunctions.at(typeIndex)(label, elementData, 0))
	{
		AddLocalChangeToEntity(entity, arrayMember.ownerTypeDesc->GetGUID(), arrayMember.name);
		EditorUtils::MarkEntityAsEdited(entity);

		return true;
	}

	return false;
}

bool ComponentPropertyUtility::DrawComponentEnum(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
	const auto& constants = enumType->GetConstants();

	int32_t& currentValue = *reinterpret_cast<int32_t*>(&bytePtr[offset + member.offset]);
	int32_t currentIndex = 0;

	std::unordered_map<int32_t, int32_t> indexToValueMap;
	Vector<std::string> constantNames;

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

		return true;
	}

	return false;
}

bool ComponentPropertyUtility::DrawComponentArray(Weak<Volt::Scene> scene, Volt::Entity entity, const Volt::ComponentMember& member, const Volt::IArrayTypeDesc* arrayDesc, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
	void* arrayPtr = &bytePtr[offset];

	const Volt::ICommonTypeDesc* elementTypeDesc = arrayDesc->GetElementTypeDesc();
	const bool isNonDefaultType = elementTypeDesc != nullptr;
	const auto& elementTypeIndex = arrayDesc->GetElementTypeIndex();

	ImGui::TableNextColumn();
	const bool arrayOpen = ImGui::TreeNodeEx(member.label.data(), ImGuiTreeNodeFlags_SpanFullWidth);

	bool edited = false;

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
						edited |= DrawComponent(scene, entity, compDesc, elementData, 0, open, true);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(elementTypeDesc);
						edited |= DrawComponentEnum(scene, entity, member, enumDesc, elementData, 0);
						break;
					}

					case Volt::ValueType::Array:
					{
						const Volt::IArrayTypeDesc* elementArrayDesc = reinterpret_cast<const Volt::IArrayTypeDesc*>(elementTypeDesc);
						edited |= DrawComponentArray(scene, entity, member, elementArrayDesc, elementData, 0);
						break;
					}
				}
			}
			else
			{
				edited |= DrawComponentDefaultMemberArray(scene, entity, member, elementData, i, elementTypeIndex, member.GetAssetType());
			}
		}

		if (ImGui::Button((std::string("Add##add_") + std::string(member.label)).c_str()))
		{
			arrayDesc->EmplaceBack(arrayPtr, nullptr);
			AddLocalChangeToEntity(entity, member.ownerTypeDesc->GetGUID(), member.name);

			EditorUtils::MarkEntityAsEdited(entity);
			edited = true;
		}

		ImGui::TreePop();
	}

	if (!arrayOpen)
	{
		ImGui::TableNextColumn();
	}

	return edited;
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
