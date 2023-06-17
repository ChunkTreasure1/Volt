#pragma once

#include <Volt/Utility/UIUtility.h>

#include <glm/glm.hpp>

namespace EditorTheme
{
	inline static glm::vec4 ToNormalizedRGB(float r, float g, float b, float a = 255.f)
	{
		return { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
	}

	inline static ImVec4 ToImVec4(const glm::vec4& color)
	{
		return ImVec4{ color.x, color.y, color.z, color.w };
	}

	inline static const glm::vec4 DarkBackground = { 0.2f, 0.2f, 0.2f, 1.f };
	inline static const glm::vec4 MatchingTextBackground = ToNormalizedRGB(139.f, 194.f, 74.f);

	inline static const glm::vec4 ItemHovered = { 0.35f, 0.35f, 0.35f, 1.f };
	inline static const glm::vec4 ItemSelectedFocused = { 0.f, 0.44f, 1.f, 1.f };
	inline static const glm::vec4 ItemSelected = { 0.3f, 0.54f, 0.8f, 1.f };
	inline static const glm::vec4 ItemChildActive = { 0.17f, 0.196f, 0.227f, 1.f };

	inline static const glm::vec4 ItemIsPrefab = ToNormalizedRGB(56.f, 156.f, 255.f);
	inline static const glm::vec4 ItemIsInvalidPrefab = { 1.f, 1.f, 0.f, 1.f };
	inline static const glm::vec4 SceneLayerBackground = { 0.12f, 0.12f, 0.12f, 1.f };
	inline static const glm::vec4 HighlightedText = { 0.4f, 0.67f, 1.f, 1.f };

	namespace Buttons
	{
		inline static const UI::Button RemoveButton = { { 0.8f, 0.1f, 0.15f, 1.f }, { 0.9f, 0.2f, 0.2f, 1.f }, { 0.8f, 0.1f, 0.15f, 1.f } };
		inline static const UI::Button AddButton = { { 0.f, 0.78f, 0.25f, 1.f }, { 0.f, 0.98f, 0.31f, 1.f }, { 0.f, 0.78f, 0.25f, 1.f } };
	}
}
