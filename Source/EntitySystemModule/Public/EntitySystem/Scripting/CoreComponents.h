#pragma once

#include "EntitySystem/ComponentRegistry.h"
#include "EntitySystem/EntityID.h"

#include <glm/glm.hpp>

namespace Volt
{
	enum class Movability : uint32_t
	{
		Static = 0,
		Stationary,
		Movable
	};

	static void ReflectType(TypeDesc<Movability>& reflect)
	{
		reflect.SetGUID("{2DC55D4C-63C8-432A-8037-1D6762C8DC33}"_guid);
		reflect.SetLabel("Movability");
		reflect.SetDefaultValue(Movability::Static);
		reflect.AddConstant(Movability::Static, "static", "Static");
		reflect.AddConstant(Movability::Stationary, "stationary", "Stationary");
		reflect.AddConstant(Movability::Movable, "movable", "Movable");
	}

	REGISTER_ENUM(Movability);

	struct VTES_API TagComponent
	{
		std::string tag;

		static void ReflectType(TypeDesc<TagComponent>& reflect)
		{
			reflect.SetGUID("{282FA5FB-6A77-47DB-8340-3D34F1A1FBBD}"_guid);
			reflect.SetLabel("Tag Component");
			reflect.SetHidden();
			reflect.AddMember(&TagComponent::tag, "tag", "Tag", "", std::string(""));
		}

		REGISTER_COMPONENT(TagComponent);
	};

	struct VTES_API IDComponent
	{
		EntityID id{};

		static void ReflectType(TypeDesc<IDComponent>& reflect)
		{
			reflect.SetGUID("{663E0E0B-43EC-4973-8A9B-FF8A0BA566AA}"_guid);
			reflect.SetLabel("ID Component");
			reflect.SetHidden();
			reflect.AddMember(&IDComponent::id, "id", "ID", "", EntityID{}, ComponentMemberFlag::NoSerialize);
		}

		REGISTER_COMPONENT(IDComponent);
	};

	struct VTES_API TransformComponent
	{
		glm::vec3 position = { 0.f };
		glm::quat rotation = glm::identity<glm::quat>();
		glm::vec3 scale = { 1.f };

		Movability movability = Movability::Static;

		bool visible = true;
		bool locked = false;

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

		static void ReflectType(TypeDesc<TransformComponent>& reflect)
		{
			reflect.SetGUID("{E1B8016B-1CAA-4782-927E-C17C29B25893}"_guid);
			reflect.SetLabel("Transform Component");
			reflect.SetHidden();
			reflect.AddMember(&TransformComponent::position, "position", "Position", "", glm::vec3{ 0.f });
			reflect.AddMember(&TransformComponent::rotation, "rotation", "Rotation", "", glm::identity<glm::quat>());
			reflect.AddMember(&TransformComponent::scale, "scale", "Scale", "", glm::vec3{ 1.f });
			reflect.AddMember(&TransformComponent::visible, "visible", "Visible", "", true);
			reflect.AddMember(&TransformComponent::locked, "locked", "Locked", "", false);
			reflect.AddMember(&TransformComponent::movability, "movability", "Movability", "", Movability::Static);
		}

		REGISTER_COMPONENT(TransformComponent);
	};

	struct VTES_API RelationshipComponent
	{
		EntityID parent = EntityID(0);
		Vector<EntityID> children;

		static void ReflectType(TypeDesc<RelationshipComponent>& reflect)
		{
			reflect.SetGUID("{4A5FEDD2-4D0B-4696-A9E6-DCDFFB25B32C}"_guid);
			reflect.SetLabel("Relationship Component");
			reflect.SetHidden();
			reflect.AddMember(&RelationshipComponent::parent, "parent", "Parent", "", EntityID(0));
			reflect.AddMember(&RelationshipComponent::children, "children", "Children", "", Vector<EntityID>{});
		}

		REGISTER_COMPONENT(RelationshipComponent);
	};
}
