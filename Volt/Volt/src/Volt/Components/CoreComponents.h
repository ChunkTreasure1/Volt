#pragma once

#include "Volt/Scene/Serialization/ComponentReflection.h"
#include "Volt/Scene/Serialization/ComponentRegistry.h"

#include <glm/glm.hpp>

#include <string>
#include <string_view>

namespace Volt
{
	struct EnTTCommonComponent
	{
		uint32_t layerId = 0;

		static void Reflect(ComponentDesc<EnTTCommonComponent>& reflect)
		{
			reflect.SetGUID("{94D13673-10AB-404A-84E3-1E7FB61C0346}"_v_guid);
			reflect.SetLabel("Common Component");
			reflect.AddMember(&EnTTCommonComponent::layerId, 0, "layerid", "Layer ID", "", 0u);
		}

		REGISTER_COMPONENT(EnTTCommonComponent);
	};

	struct EnTTTagComponent
	{
		EnTTTagComponent(std::string_view inTag) : tag(inTag) {}
		std::string tag;

		static void Reflect(ComponentDesc<EnTTTagComponent>& reflect)
		{
			reflect.SetGUID("{CE3F8A13-C73A-4D61-AF93-AA12036FBBFB}"_v_guid);
			reflect.SetLabel("Tag Component");
			reflect.AddMember(&EnTTTagComponent::tag, 0, "tag", "Tag", "", std::string(""));
		}

		REGISTER_COMPONENT(EnTTTagComponent);
	};

	struct EnTTTransformComponent
	{
		glm::vec3 position = { 0.f };
		glm::quat rotation = glm::identity<glm::quat>();
		glm::vec3 scale = { 1.f };

		bool visible = false;
		bool locked = true;

		inline const glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.f), position) *
				glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.f), scale);
		}

		inline const glm::vec3 GetForward() const
		{
			return glm::rotate(rotation, glm::vec3{ 0.f, 0.f, 1.f });
		}

		inline const glm::vec3 GetRight() const
		{
			return glm::rotate(rotation, glm::vec3{ 1.f, 0.f, 0.f });
		}

		inline const glm::vec3 GetUp() const
		{
			return glm::rotate(rotation, glm::vec3{ 0.f, 1.f, 0.f });
		}

		static void Reflect(ComponentDesc<EnTTTransformComponent>& reflect)
		{
			reflect.SetGUID("{5D680EA7-BED3-44BB-A47D-EA5D9B944ACE}"_v_guid);
			reflect.SetLabel("Transform Component");
			reflect.AddMember(&EnTTTransformComponent::position, 0, "position", "Position", "", glm::vec3{ 0.f });
			reflect.AddMember(&EnTTTransformComponent::rotation, 1, "rotation", "Rotation", "", glm::identity<glm::quat>());
			reflect.AddMember(&EnTTTransformComponent::scale, 2, "scale", "Scale", "", glm::vec3{ 1.f });
			reflect.AddMember(&EnTTTransformComponent::visible, 3, "visible", "Visible", "", true);
			reflect.AddMember(&EnTTTransformComponent::locked, 4, "locked", "Locked", "", false);
		}

		REGISTER_COMPONENT(EnTTTransformComponent);
	};

	struct EnTTRelationshipComponent
	{
		entt::entity parent = entt::null;
		std::vector<entt::entity> children;

		static void Reflect(ComponentDesc<EnTTRelationshipComponent>& reflect)
		{
			reflect.SetGUID("{82FE1C3B-5327-4307-9A13-3075A13BA81B}"_v_guid);
			reflect.SetLabel("Relationship Component");
			reflect.AddMember(&EnTTRelationshipComponent::parent, 0, "parent", "Parent", "", entt::null);
			reflect.AddMember(&EnTTRelationshipComponent::children, 1, "children", "Children", "", std::vector<entt::entity>{});
		}

		REGISTER_COMPONENT(EnTTRelationshipComponent);
	};
}
