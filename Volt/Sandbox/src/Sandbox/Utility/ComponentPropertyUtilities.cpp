#include "sbpch.h"
#include "ComponentPropertyUtilities.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Scene/Reflection/ComponentReflection.h>
#include <Volt/Scene/Reflection/ComponentRegistry.h>

#include <Volt/Components/LightComponents.h>

#include <Volt/Utility/UIUtility.h>

#include <glm/glm.hpp>

template<typename T>
void RegisterPropertyType(std::unordered_map<std::type_index, std::function<bool(std::string_view, void*, const size_t)>>& outFunctionMap)
{
	outFunctionMap[std::type_index{ typeid(T) }] = [](std::string_view label, void* data, const size_t offset) -> bool { uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data); return UI::Property(std::string(label), *reinterpret_cast<T*>(&bytePtr[offset])); };
}

void ComponentPropertyUtility::Initialize()
{
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

void ComponentPropertyUtility::DrawComponentProperties(Weak<Volt::Scene> scene, Volt::Entity entity)
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

	for (uint32_t index = 0; const auto& constant : constants)
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
