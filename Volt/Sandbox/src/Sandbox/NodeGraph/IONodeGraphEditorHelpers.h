#pragma once

#include <Volt/Scene/Entity.h>
#include <Volt/Components/Components.h>
#include <Volt/Asset/AssetManager.h>
#include <Volt/Utility/UIUtility.h>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <sstream>
#include <string>
#include <string_view>
#include <typeindex>
#include <iomanip>

template<typename T>
inline static constexpr std::type_index GetTypeIndex()
{
	return std::type_index(typeid(T));
}

class IONodeGraphEditorHelpers
{
public:
	inline static void Initialize()
	{
		// Functions
		{
			myAttributeFunctions[GetTypeIndex<bool>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<bool&>(value)); };
			myAttributeFunctions[GetTypeIndex<int32_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<int32_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<uint32_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<uint32_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<int16_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<int16_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<uint16_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<uint16_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<int8_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<int8_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<uint8_t>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<uint8_t&>(value)); };
			myAttributeFunctions[GetTypeIndex<double>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<double&>(value)); };
			myAttributeFunctions[GetTypeIndex<float>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<float&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::vec2>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::vec2&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::vec3>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::vec3&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::vec4>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::vec4&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::uvec2>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::uvec2&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::uvec3>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::uvec3&>(value)); };
			myAttributeFunctions[GetTypeIndex<glm::uvec4>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<glm::uvec4&>(value)); };
			myAttributeFunctions[GetTypeIndex<std::string>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<std::string&>(value)); };

			myAttributeFunctions[GetTypeIndex<Volt::Entity>()] = [](std::any& value) { IONodeGraphEditorHelpers::Attribute(std::any_cast<Volt::Entity&>(value)); };
		}

		// Colors
		{
			///// Colors //////
			// Bool: Dark red
			// Int: Green
			// Float: Blue
			// String: Magenta
			// VectorN: Yellow
			// Default: Orange
			///////////////////

			myAttributeColors[GetTypeIndex<bool>()] = { 0.58f, 0.f, 0.01f, 1.f };

			myAttributeColors[GetTypeIndex<int32_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };
			myAttributeColors[GetTypeIndex<uint32_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };
			myAttributeColors[GetTypeIndex<int16_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };
			myAttributeColors[GetTypeIndex<uint16_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };
			myAttributeColors[GetTypeIndex<int8_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };
			myAttributeColors[GetTypeIndex<uint8_t>()] = { 0.11f, 0.64f, 0.5f, 1.f };

			myAttributeColors[GetTypeIndex<double>()] = { 0.2f, 0.72f, 0.011f, 1.f };
			myAttributeColors[GetTypeIndex<float>()] = { 0.2f, 0.72f, 0.011f, 1.f };

			myAttributeColors[GetTypeIndex<glm::vec2>()] = { 0.88f, 0.7f, 0.13f, 1.f };
			myAttributeColors[GetTypeIndex<glm::vec3>()] = { 0.88f, 0.7f, 0.13f, 1.f };
			myAttributeColors[GetTypeIndex<glm::vec4>()] = { 0.88f, 0.7f, 0.13f, 1.f };
			myAttributeColors[GetTypeIndex<glm::quat>()] = { 0.88f, 0.7f, 0.13f, 1.f };

			myAttributeColors[GetTypeIndex<glm::uvec2>()] = { 0.88f, 0.7f, 0.13f, 1.f };
			myAttributeColors[GetTypeIndex<glm::uvec3>()] = { 0.88f, 0.7f, 0.13f, 1.f };
			myAttributeColors[GetTypeIndex<glm::uvec4>()] = { 0.88f, 0.7f, 0.13f, 1.f };

			myAttributeColors[GetTypeIndex<std::string>()] = { 0.83f, 0.023f, 0.7f, 1.f };

			myAttributeColors[GetTypeIndex<Volt::Entity>()] = { 0.3f, 1.f, 0.49f, 1.f };

			myDefaultPinColor = { 0.99f, 0.51f, 0.f, 1.f };
		}
	}

	inline static void Shutdown()
	{

	}

	inline static void BeginAttributes()
	{
		ImGui::PushID(s_contextId++);
		s_stackId = 0;
	}

	inline static void EndAttributes()
	{
		ImGui::PopID();
		s_contextId--;
	}

	inline static void Attribute(bool& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::Checkbox(id.c_str(), &value);
	}

	inline static void Attribute(int32_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_S32, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(uint32_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(int16_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_S16, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(uint16_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_U16, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(int8_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_S8, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(uint8_t& value)
	{
		const ImVec2 width = ImGui::CalcTextSize(std::to_string(value).c_str());

		ImGui::PushItemWidth(width.x + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_U8, &value);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(double& value)
	{
		std::ostringstream oss;
		oss << std::setprecision(1) << value;

		const ImVec2 width = ImGui::CalcTextSize(oss.str().c_str());

		ImGui::PushItemWidth(std::max(width.x, 20.f) + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_Double, &value, 1.f, nullptr, nullptr, "%.1f");

		ImGui::PopItemWidth();
	}

	inline static void Attribute(float& value)
	{
		std::ostringstream oss;
		oss << std::setprecision(1) << value;

		const ImVec2 width = ImGui::CalcTextSize(oss.str().c_str());

		ImGui::PushItemWidth(std::max(width.x, 20.f) + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value, 1.f, nullptr, nullptr, "%.3f");

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::vec2& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.x, 1.f, nullptr, nullptr, "%.1f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.y, 1.f, nullptr, nullptr, "%.1f");
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::vec3& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.x, 1.f, nullptr, nullptr, "%.3f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.y, 1.f, nullptr, nullptr, "%.3f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.z, 1.f, nullptr, nullptr, "%.3f");
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::vec4& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.x, 1.f, nullptr, nullptr, "%.1f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.y, 1.f, nullptr, nullptr, "%.1f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.z, 1.f, nullptr, nullptr, "%.1f");
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value.w, 1.f, nullptr, nullptr, "%.1f");
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::uvec2& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.x);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.y);
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::uvec3& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.x);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.y);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.z);
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(glm::uvec4& value)
	{
		ImGui::PushItemWidth(40.f);

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.x);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.y);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.z);
		}

		{
			const std::string id = "##" + std::to_string(s_stackId++);
			ImGui::DragScalar(id.c_str(), ImGuiDataType_U32, &value.w);
		}

		ImGui::PopItemWidth();
	}

	inline static void Attribute(std::string& value)
	{
		ImGui::PushItemWidth(50.f);
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::InputTextString(id.c_str(), &value);
		ImGui::PopItemWidth();
	}

	inline static void Attribute(Volt::Entity& entity)
	{
		std::string entityName;
		if (entity)
		{
			entityName = entity.GetComponent<Volt::TagComponent>().tag;
		}
		else
		{
			entityName = "Null";
		}

		const ImVec2 width = ImGui::CalcTextSize(entityName.c_str());
		ImGui::PushItemWidth(std::max(width.x, 20.f) + ATTR_PADDING);

		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::InputTextString(id.c_str(), &entityName, ImGuiInputTextFlags_ReadOnly);

		ImGui::PopItemWidth();
	}

	inline static void Attribute(Volt::AssetHandle& assetHandle, Volt::AssetType supportedTypes)
	{
		std::string assetFileName = "Null";

		const Ref<Volt::Asset> rawAsset = Volt::AssetManager::Get().GetAssetRaw(assetHandle);
		if (rawAsset)
		{
			assetFileName = rawAsset->assetName;

			if (supportedTypes != Volt::AssetType::None)
			{
				if ((rawAsset->GetType() & supportedTypes) == Volt::AssetType::None)
				{
					assetHandle = Volt::Asset::Null();
				}
			}
		}

		const ImVec2 width = ImGui::CalcTextSize(assetFileName.c_str());
		ImGui::PushItemWidth(std::max(width.x, 20.f) + 5.f);

		const std::string id = "##" + std::to_string(IONodeGraphEditorHelpers::GetId());
		ImGui::InputTextString(id.c_str(), &assetFileName, ImGuiInputTextFlags_ReadOnly);
		ImGui::PopItemWidth();

		if (auto ptr = UI::DragDropTarget("ASSET_BROWSER_ITEM"))
		{
			Volt::AssetHandle newHandle = *(Volt::AssetHandle*)ptr;
			assetHandle = newHandle;
		}
	}

	inline static const auto& GetAttribFunctions() { return myAttributeFunctions; }
	inline static const auto& GetAttribColors() { return myAttributeColors; }
	inline static const auto& GetDefaultPinColor() { return myDefaultPinColor; }
	inline static const uint32_t GetId() { return s_stackId++; }

private:
	inline static uint32_t s_contextId = 0;
	inline static uint32_t s_stackId = 0;

	inline static constexpr float ATTR_PADDING = 5.f;

	inline static std::unordered_map<std::type_index, std::function<void(std::any& data)>> myAttributeFunctions;
	inline static std::unordered_map<std::type_index, glm::vec4> myAttributeColors;
	inline static glm::vec4 myDefaultPinColor;

	IONodeGraphEditorHelpers() = delete;
};
