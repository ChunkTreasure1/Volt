#pragma once

#include "Volt/Scene/ComponentRegistry.h"

#include <glm/glm.hpp>

#include <string>
#include <string_view>

namespace Volt
{
	struct EnTTTagComponent
	{
		EnTTTagComponent(std::string_view inTag) : tag(inTag) {}
		std::string tag;

		ENTT_COMPONENT(EnTTTagComponent);
		ENTT_PROPERTY(EnTTTagComponent, tag, "Tag");
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

		ENTT_COMPONENT(EnTTTransformComponent);
		ENTT_PROPERTY(EnTTTransformComponent, position, "Position");
		ENTT_PROPERTY(EnTTTransformComponent, rotation, "Rotation");
		ENTT_PROPERTY(EnTTTransformComponent, scale, "Scale");
		ENTT_PROPERTY(EnTTTransformComponent, visible, "Visible");
		ENTT_PROPERTY(EnTTTransformComponent, locked, "Locked");
	};

	struct EnTTRelationshipComponent
	{
		entt::entity parent = entt::null;
		std::vector<entt::entity> children;

		ENTT_COMPONENT(EnTTRelationshipComponent);
		ENTT_PROPERTY(EnTTRelationshipComponent, parent, "Parent");
		ENTT_PROPERTY(EnTTRelationshipComponent, children, "Children");
	};
}
