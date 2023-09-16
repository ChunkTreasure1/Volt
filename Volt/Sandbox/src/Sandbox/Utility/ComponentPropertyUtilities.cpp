#include "sbpch.h"
#include "ComponentPropertyUtilities.h"

#include "Sandbox/Utility/Theme.h"
#include "Sandbox/Utility/EditorUtilities.h"

#include <Volt/Scene/Serialization/ComponentReflection.h>
#include <Volt/Scene/Serialization/ComponentRegistry.h>

#include <Volt/Utility/UIUtility.h>

#include <glm/glm.hpp>

template<typename T>
void RegisterPropertyType(std::unordered_map<std::type_index, std::function<bool(std::string_view, void*, const size_t)>>& outFunctionMap)
{
	outFunctionMap[std::type_index{ typeid(T) }] = [](std::string_view label, void* data, const size_t offset) -> bool { uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data); return UI::Property(std::string(label), *reinterpret_cast<T*>(bytePtr + offset)); };
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
					DrawComponent(compTypeDesc, storage.get(entity.GetID()));
					break;
				}
			}
		}
	}
}

void ComponentPropertyUtility::DrawComponent(const Volt::IComponentTypeDesc* componentType, void* data)
{
	if (componentType->IsHidden())
	{
		return;
	}

	bool removeComp = false;
	bool open = UI::CollapsingHeader(componentType->GetLabel());
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

	std::string id = "-###Remove" + std::string(componentType->GetLabel());

	{
		UI::ScopedStyleFloat round{ ImGuiStyleVar_FrameRounding, 0.f };
		UI::ScopedStyleFloat2 pad{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };
		UI::ScopedButtonColor buttonColor{ EditorTheme::Buttons::RemoveButton };

		if (ImGui::Button(id.c_str(), ImVec2{ buttonSize, buttonSize }))
		{
			removeComp = true;
		}
	}

	if (open && UI::BeginProperties(std::string(componentType->GetLabel())))
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
						DrawComponentSubSection(compDesc, data, member.offset);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(member.typeDesc);
						DrawComponentEnum(member, enumDesc, data, member.offset);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMember(member, data, member.offset);
			}
		}

		UI::EndProperties();
	}
}

void ComponentPropertyUtility::DrawComponentSubSection(const Volt::IComponentTypeDesc* componentType, void* data, const size_t offset)
{
	if (componentType->IsHidden())
	{
		return;
	}

	bool open = UI::CollapsingHeader(componentType->GetLabel());
	if (open && UI::BeginProperties(std::string(componentType->GetLabel())))
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
						DrawComponentSubSection(compDesc, data, offset + member.offset);

						break;
					}

					case Volt::ValueType::Enum:
					{
						const Volt::IEnumTypeDesc* enumDesc = reinterpret_cast<const Volt::IEnumTypeDesc*>(member.typeDesc);
						DrawComponentEnum(member, enumDesc, data, offset + member.offset);
						break;
					}
				}
			}
			else
			{
				DrawComponentDefaultMember(member, data, offset);
			}
		}

		UI::EndProperties();
	}
}

void ComponentPropertyUtility::DrawComponentDefaultMember(const Volt::ComponentMember& member, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);

	if (member.assetType != Volt::AssetType::None)
	{
		EditorUtils::Property(std::string(member.label), *reinterpret_cast<Volt::AssetHandle*>(&bytePtr[offset + member.offset]), member.assetType);
		return;
	}

	if (!s_propertyFunctions.contains(member.typeIndex))
	{
		return;
	}

	s_propertyFunctions.at(member.typeIndex)(member.label, data, offset + member.offset);
}

void ComponentPropertyUtility::DrawComponentEnum(const Volt::ComponentMember& member, const Volt::IEnumTypeDesc* enumType, void* data, const size_t offset)
{
	uint8_t* bytePtr = reinterpret_cast<uint8_t*>(data);
	const auto& constants = enumType->GetConstants();

	int32_t& currentValue = *reinterpret_cast<int32_t*>(&bytePtr[offset]);
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
