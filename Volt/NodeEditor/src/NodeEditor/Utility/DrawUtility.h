#pragma once

#include "NodeEditor/Node.h"

#include <imgui_internal.h>

namespace NE
{
	enum class IconType : ImU32
	{
		Flow,
		Circle,
		Square,
		Grid,
		RoundSquare,
		Diamond
	};

	inline void DrawIcon(ImDrawList* drawList, const ImVec2& a, const ImVec2& b, IconType type, bool filled, ImU32 color, ImU32 innerColor)
	{
		auto rect = ImRect(a, b);
		auto rect_x = rect.Min.x;
		auto rect_y = rect.Min.y;
		auto rect_w = rect.Max.x - rect.Min.x;
		auto rect_h = rect.Max.y - rect.Min.y;
		auto rect_center_x = (rect.Min.x + rect.Max.x) * 0.5f;
		auto rect_center_y = (rect.Min.y + rect.Max.y) * 0.5f;
		auto rect_center = ImVec2(rect_center_x, rect_center_y);
		const auto outline_scale = rect_w / 24.0f;
		const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle

		if (type == IconType::Flow)
		{
			const auto origin_scale = rect_w / 24.0f;

			const auto offset_x = 1.0f * origin_scale;
			const auto offset_y = 0.0f * origin_scale;
			const auto margin = (filled ? 2.0f : 2.0f) * origin_scale;
			const auto rounding = 0.1f * origin_scale;
			const auto tip_round = 0.7f; // percentage of triangle edge (for tip)
			//const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
			const auto canvas = ImRect(
				rect.Min.x + margin + offset_x,
				rect.Min.y + margin + offset_y,
				rect.Max.x - margin + offset_x,
				rect.Max.y - margin + offset_y);
			const auto canvas_x = canvas.Min.x;
			const auto canvas_y = canvas.Min.y;
			const auto canvas_w = canvas.Max.x - canvas.Min.x;
			const auto canvas_h = canvas.Max.y - canvas.Min.y;

			const auto left = canvas_x + canvas_w * 0.5f * 0.3f;
			const auto right = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
			const auto top = canvas_y + canvas_h * 0.5f * 0.2f;
			const auto bottom = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
			const auto center_y = (top + bottom) * 0.5f;
			//const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

			const auto tip_top = ImVec2(canvas_x + canvas_w * 0.5f, top);
			const auto tip_right = ImVec2(right, center_y);
			const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

			drawList->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
			drawList->PathBezierCurveTo(
				ImVec2(left, top),
				ImVec2(left, top),
				ImVec2(left, top) + ImVec2(rounding, 0));
			drawList->PathLineTo(tip_top);
			drawList->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
			drawList->PathBezierCurveTo(
				tip_right,
				tip_right,
				tip_bottom + (tip_right - tip_bottom) * tip_round);
			drawList->PathLineTo(tip_bottom);
			drawList->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
			drawList->PathBezierCurveTo(
				ImVec2(left, bottom),
				ImVec2(left, bottom),
				ImVec2(left, bottom) - ImVec2(0, rounding));

			if (!filled)
			{
				if (innerColor & 0xFF000000)
					drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

				drawList->PathStroke(color, true, 2.0f * outline_scale);
			}
			else
				drawList->PathFillConvex(color);
		}
		else
		{
			auto triangleStart = rect_center_x + 0.32f * rect_w;

			auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

			rect.Min.x += rect_offset;
			rect.Max.x += rect_offset;
			rect_x += rect_offset;
			rect_center_x += rect_offset * 0.5f;
			rect_center.x += rect_offset * 0.5f;

			if (type == IconType::Circle)
			{
				const auto c = rect_center;

				if (!filled)
				{
					const auto r = 0.5f * rect_w / 2.0f - 0.5f;

					if (innerColor & 0xFF000000)
						drawList->AddCircleFilled(c, r, innerColor, 12 + extra_segments);
					drawList->AddCircle(c, r, color, 12 + extra_segments, 2.0f * outline_scale);
				}
				else
					drawList->AddCircleFilled(c, 0.5f * rect_w / 2.0f, color, 12 + extra_segments);
			}

			if (type == IconType::Square)
			{
				if (filled)
				{
					const auto r = 0.5f * rect_w / 2.0f;
					const auto p0 = rect_center - ImVec2(r, r);
					const auto p1 = rect_center + ImVec2(r, r);

					drawList->AddRectFilled(p0, p1, color, 0, 15 + extra_segments);
				}
				else
				{
					const auto r = 0.5f * rect_w / 2.0f - 0.5f;
					const auto p0 = rect_center - ImVec2(r, r);
					const auto p1 = rect_center + ImVec2(r, r);

					if (innerColor & 0xFF000000)
						drawList->AddRectFilled(p0, p1, innerColor, 0, 15 + extra_segments);

					drawList->AddRect(p0, p1, color, 0, 15 + extra_segments, 2.0f * outline_scale);
				}
			}

			if (type == IconType::Grid)
			{
				const auto r = 0.5f * rect_w / 2.0f;
				const auto w = ceilf(r / 3.0f);

				const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
				const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

				auto tl = baseTl;
				auto br = baseBr;
				for (int i = 0; i < 3; ++i)
				{
					tl.x = baseTl.x;
					br.x = baseBr.x;
					drawList->AddRectFilled(tl, br, color);
					tl.x += w * 2;
					br.x += w * 2;
					if (i != 1 || filled)
						drawList->AddRectFilled(tl, br, color);
					tl.x += w * 2;
					br.x += w * 2;
					drawList->AddRectFilled(tl, br, color);

					tl.y += w * 2;
					br.y += w * 2;
				}

				triangleStart = br.x + w + 1.0f / 24.0f * rect_w;
			}

			if (type == IconType::RoundSquare)
			{
				if (filled)
				{
					const auto r = 0.5f * rect_w / 2.0f;
					const auto cr = r * 0.5f;
					const auto p0 = rect_center - ImVec2(r, r);
					const auto p1 = rect_center + ImVec2(r, r);

					drawList->AddRectFilled(p0, p1, color, cr, 15);
				}
				else
				{
					const auto r = 0.5f * rect_w / 2.0f - 0.5f;
					const auto cr = r * 0.5f;
					const auto p0 = rect_center - ImVec2(r, r);
					const auto p1 = rect_center + ImVec2(r, r);

					if (innerColor & 0xFF000000)
						drawList->AddRectFilled(p0, p1, innerColor, cr, 15);

					drawList->AddRect(p0, p1, color, cr, 15, 2.0f * outline_scale);
				}
			}
			else if (type == IconType::Diamond)
			{
				if (filled)
				{
					const auto r = 0.607f * rect_w / 2.0f;
					const auto c = rect_center;

					drawList->PathLineTo(c + ImVec2(0, -r));
					drawList->PathLineTo(c + ImVec2(r, 0));
					drawList->PathLineTo(c + ImVec2(0, r));
					drawList->PathLineTo(c + ImVec2(-r, 0));
					drawList->PathFillConvex(color);
				}
				else
				{
					const auto r = 0.607f * rect_w / 2.0f - 0.5f;
					const auto c = rect_center;

					drawList->PathLineTo(c + ImVec2(0, -r));
					drawList->PathLineTo(c + ImVec2(r, 0));
					drawList->PathLineTo(c + ImVec2(0, r));
					drawList->PathLineTo(c + ImVec2(-r, 0));

					if (innerColor & 0xFF000000)
						drawList->AddConvexPolyFilled(drawList->_Path.Data, drawList->_Path.Size, innerColor);

					drawList->PathStroke(color, true, 2.0f * outline_scale);
				}
			}
			else
			{
				const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

				drawList->AddTriangleFilled(
					ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
					ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
					ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
					color);
			}
		}
	}

	inline const ImColor GetIconColor(PinType type)
	{
		switch (type)
		{
			default:
			case PinType::Flow:     return ImColor(255, 255, 255);
			case PinType::Bool:     return ImColor(220, 48, 48);
			case PinType::Int:      return ImColor(68, 201, 156);
			case PinType::Float:    return ImColor(147, 226, 74);
			case PinType::String:   return ImColor(124, 21, 153);
			case PinType::Object:   return ImColor(51, 150, 215);
			case PinType::Function: return ImColor(218, 0, 183);
			case PinType::Delegate: return ImColor(255, 48, 48);

			case PinType::Framebuffer: return ImColor(66, 135, 245);
			case PinType::ColorAttachment: return ImColor(192, 38, 33);
			case PinType::DepthAttachment: return ImColor(94, 17, 14);
		}

		return ImColor(1.f, 1.f, 1.f);
	}

	inline void DrawPinIcon(const Pin& pin, bool connected, int32_t alpha)
	{
		IconType iconType;
		ImColor  color = GetIconColor(pin.type);
		color.Value.w = alpha / 255.0f;
		switch (pin.type)
		{
			case PinType::Flow:     iconType = IconType::Flow;   break;
			case PinType::Bool:     iconType = IconType::Circle; break;
			case PinType::Int:      iconType = IconType::Circle; break;
			case PinType::Float:    iconType = IconType::Circle; break;
			case PinType::String:   iconType = IconType::Circle; break;
			case PinType::Object:   iconType = IconType::Circle; break;
			case PinType::Function: iconType = IconType::Circle; break;
			case PinType::Delegate: iconType = IconType::Square; break;
			
			case PinType::Framebuffer: iconType = IconType::Square; break;
			case PinType::ColorAttachment: iconType = IconType::Circle; break;
			case PinType::DepthAttachment: iconType = IconType::Circle; break;
			
			default:
				return;
		}

		constexpr int32_t iconSize = 24;
		const ImVec2 size = ImVec2(iconSize, iconSize);

		if (ImGui::IsRectVisible(size))
		{
			auto cursorPos = ImGui::GetCursorScreenPos();
			auto drawList = ImGui::GetWindowDrawList();

			DrawIcon(drawList, cursorPos, cursorPos + size, iconType, connected, ImColor(color), ImColor(32, 32, 32, alpha));
		}

		ImGui::Dummy(size);
	}
}