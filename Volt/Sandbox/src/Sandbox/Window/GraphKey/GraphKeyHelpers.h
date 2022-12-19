#pragma once

#include <GEM/gem.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <string>
#include <string_view>

class GraphKeyHelpers
{
public:
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
		ImGui::DragScalar(id.c_str(), ImGuiDataType_Float, &value, 1.f, nullptr, nullptr, "%.1f");
		
		ImGui::PopItemWidth();
	}

	inline static void Attribute(gem::vec2& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragFloat2(id.c_str(), gem::value_ptr(value), 1.f, 0.f, 0.f, "%.1f");
	}

	inline static void Attribute(gem::vec3& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragFloat3(id.c_str(), gem::value_ptr(value), 1.f, 0.f, 0.f, "%.1f");
	}

	inline static void Attribute(gem::vec4& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragFloat4(id.c_str(), gem::value_ptr(value), 1.f, 0.f, 0.f, "%.1f");
	}

	inline static void Attribute(gem::vec2ui& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, gem::value_ptr(value), 2, 1.f, nullptr, nullptr, "%.1f");
	}

	inline static void Attribute(gem::vec3ui& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, gem::value_ptr(value), 3, 1.f, nullptr, nullptr, "%.1f");
	}

	inline static void Attribute(gem::vec4ui& value)
	{
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::DragScalarN(id.c_str(), ImGuiDataType_U32, gem::value_ptr(value), 4, 1.f, nullptr, nullptr, "%.1f");
	}

	inline static void Attribute(std::string& value)
	{
		ImGui::PushItemWidth(50.f);
		const std::string id = "##" + std::to_string(s_stackId++);
		ImGui::InputTextString(id.c_str(), &value);
		ImGui::PopItemWidth();
	}

private:
	inline static uint32_t s_contextId = 0;
	inline static uint32_t s_stackId = 0;

	inline static constexpr float ATTR_PADDING = 5.f;

	GraphKeyHelpers() = delete;
};