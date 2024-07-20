#pragma once

#include "Volt/Scene/Reflection/ComponentReflection.h"
#include "Volt/Scene/Reflection/ComponentRegistry.h"

#include "Volt/Scene/EntityID.h"

namespace Volt
{
	enum class UIAnchor : uint32_t
	{
		TopLeft = 0,
		TopMiddle,
		TopRight,

		MiddleLeft,
		MiddleMiddle,
		MiddleRight,

		BottomLeft,
		BottomMiddle,
		BottomRight
	};

	static void ReflectType(TypeDesc<UIAnchor>& reflect)
	{
		reflect.SetGUID("{588573D9-9CF6-460D-88CF-DD13D2F456FD}"_guid);
		reflect.SetLabel("UIAnchor");
		reflect.SetDefaultValue(UIAnchor::TopLeft);
		reflect.AddConstant(UIAnchor::TopLeft, "topleft", "Top left");
		reflect.AddConstant(UIAnchor::TopMiddle, "topmiddle", "Top Middle");
		reflect.AddConstant(UIAnchor::TopRight, "topright", "Top Right");
		
		reflect.AddConstant(UIAnchor::MiddleLeft, "middleleft", "Middle left");
		reflect.AddConstant(UIAnchor::MiddleMiddle, "middlemiddle", "Middle Middle");
		reflect.AddConstant(UIAnchor::MiddleRight, "middleright", "Middle Right");

		reflect.AddConstant(UIAnchor::BottomLeft, "bottomleft", "Bottom left");
		reflect.AddConstant(UIAnchor::BottomMiddle, "bottommiddle", "Bottom Middle");
		reflect.AddConstant(UIAnchor::BottomRight, "bottomright", "Bottom Right");
	}

	struct UIIDComponent
	{
		EntityID id{};
		uint64_t timeCreateID = 0;

		static void ReflectType(TypeDesc<UIIDComponent>& reflect)
		{
			reflect.SetGUID("{C6427474-18D0-406E-8082-A15D19EFEB43}"_guid);
			reflect.SetLabel("UI ID Component");
			reflect.SetHidden();
			reflect.AddMember(&UIIDComponent::id, "id", "ID", "", EntityID{}, ComponentMemberFlag::NoSerialize);
			reflect.AddMember(&UIIDComponent::timeCreateID, "timecreatedid", "Time Created", "", 0u);
		}

		REGISTER_COMPONENT(UIIDComponent);
	};

	struct UITransformComponent
	{
		UIAnchor anchor = UIAnchor::TopLeft;
		glm::vec2 position = { 0.f };
		glm::vec2 size = { 100.f };
		glm::vec2 alignment = { 0.f };
		float rotation = 0.f;
		int32_t zOrder = 0u;

		bool visible = true;

		static void ReflectType(TypeDesc<UITransformComponent>& reflect)
		{
			reflect.SetGUID("{2496CEA6-8D13-4BF0-8E6D-DF959E4C0C44}"_guid);
			reflect.SetLabel("UI Transform Component");
			reflect.SetHidden();
			reflect.AddMember(&UITransformComponent::anchor, "anchor", "Anchor", "", UIAnchor::TopLeft);
			reflect.AddMember(&UITransformComponent::position, "position", "Position", "", glm::vec2{ 0.f });
			reflect.AddMember(&UITransformComponent::size, "size", "Size", "", glm::vec2{ 100.f });
			reflect.AddMember(&UITransformComponent::alignment, "alignment", "Alignment", "", glm::vec2{ 0.f });
			reflect.AddMember(&UITransformComponent::rotation, "rotation", "Rotation", "", 0.f);
			reflect.AddMember(&UITransformComponent::zOrder, "zorder", "Z Order", "", 0);
			reflect.AddMember(&UITransformComponent::visible, "visible", "Visible", "", false);
		}

		REGISTER_COMPONENT(UITransformComponent);
	};

	struct UITagComponent
	{
		std::string tag;

		static void ReflectType(TypeDesc<UITagComponent>& reflect)
		{
			reflect.SetGUID("{813B1914-C653-485F-96F5-87578F6916B5}"_guid);
			reflect.SetLabel("UI Tag Component");
			reflect.SetHidden();
			reflect.AddMember(&UITagComponent::tag, "tag", "Tag", "", std::string());
		}

		REGISTER_COMPONENT(UITagComponent);
	};

	struct UIImageComponent
	{
		AssetHandle imageHandle = 0;
		glm::vec3 tint = { 1.f };
		float alpha = 1.f;

		static void ReflectType(TypeDesc<UIImageComponent>& reflect)
		{
			reflect.SetGUID("{5FAF4CED-38A8-4B80-8C6B-786651E72EEF}"_guid);
			reflect.SetLabel("UI Image Component");
			reflect.SetHidden();
			reflect.AddMember(&UIImageComponent::imageHandle, "image", "Image", "", Asset::Null(), AssetType::Texture);
			reflect.AddMember(&UIImageComponent::tint, "tint", "Tint", "", glm::vec3{ 1.f });
			reflect.AddMember(&UIImageComponent::alpha, "alpha", "Alpha", "", 1.f);
		}

		REGISTER_COMPONENT(UIImageComponent);
	};
}
