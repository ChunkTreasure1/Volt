#pragma once

#include <EntitySystem/ComponentRegistry.h>

#include <AssetSystem/Asset.h>

#include <glm/glm.hpp>

namespace Volt
{
	struct NavMeshComponent
	{
		static void ReflectType(TypeDesc<NavMeshComponent>& reflect)
		{
			reflect.SetGUID("{6643C885-4A12-4527-8AA7-7894A730BAC5}"_guid);
			reflect.SetLabel("Nav Mesh Component");
		}

		REGISTER_COMPONENT(NavMeshComponent);
	};

	struct NavLinkComponent
	{
		glm::vec3 start = { 0.f };
		glm::vec3 end = { 0.f };
		bool bidirectional = true;
		bool active = true;

		static void ReflectType(TypeDesc<NavLinkComponent>& reflect)
		{
			reflect.SetGUID("{7104A8A0-3657-4840-B172-89F5E79E64A6}"_guid);
			reflect.SetLabel("Nav Link Component");
			reflect.AddMember(&NavLinkComponent::start, "start", "Start", "", glm::vec3{ 0.f });
			reflect.AddMember(&NavLinkComponent::end, "end", "End", "", glm::vec3{ 0.f });
			reflect.AddMember(&NavLinkComponent::bidirectional, "bidirectional", "Bidirectional", "", true);
			reflect.AddMember(&NavLinkComponent::active, "active", "Active", "", true);
		}

		REGISTER_COMPONENT(NavLinkComponent);
	};

	enum class ObstacleAvoidanceQuality : uint32_t
	{
		None = 0,
		Low,
		Medium,
		High
	};

	static void ReflectType(TypeDesc<ObstacleAvoidanceQuality>& reflect)
	{
		reflect.SetGUID("{E3DFBC11-375E-49BC-A39F-B66392B3AB98}"_guid);
		reflect.SetLabel("Obstacle Avoidance Quality");
		reflect.SetDefaultValue(ObstacleAvoidanceQuality::None);
		reflect.AddConstant(ObstacleAvoidanceQuality::None, "none", "None");
		reflect.AddConstant(ObstacleAvoidanceQuality::Low, "low", "Low");
		reflect.AddConstant(ObstacleAvoidanceQuality::Medium, "medium", "Medium");
		reflect.AddConstant(ObstacleAvoidanceQuality::High, "high", "High");
	}

	REGISTER_ENUM(ObstacleAvoidanceQuality);

	struct NavAgentComponent
	{
		float radius = 60.f;
		float height = 200.f;
		float maxSpeed = 300.f;
		float acceleration = 1000.f;
		float separationWeight = 0.f;
		ObstacleAvoidanceQuality obstacleAvoidanceQuality = ObstacleAvoidanceQuality::None;
		bool active = true;

		static void ReflectType(TypeDesc<NavAgentComponent>& reflect)
		{
			reflect.SetGUID("{2B4469CE-9B15-4FA9-ABA6-77BA83465357}"_guid);
			reflect.SetLabel("Nav Agent Component");
			reflect.AddMember(&NavAgentComponent::radius, "radius", "Radius", "", 60.f);
			reflect.AddMember(&NavAgentComponent::height, "height", "Height", "", 200.f);
			reflect.AddMember(&NavAgentComponent::maxSpeed, "maxSpeed", "Max Speed", "", 300.f);
			reflect.AddMember(&NavAgentComponent::acceleration, "acceleration", "Acceleration", "", 1000.f);
			reflect.AddMember(&NavAgentComponent::separationWeight, "seperationWeight", "Seperation Weight", "", 0.f);
			reflect.AddMember(&NavAgentComponent::obstacleAvoidanceQuality, "obstacleAvoidanceQuality", "Obstacle Avoidance Quality", "", ObstacleAvoidanceQuality::None);
			reflect.AddMember(&NavAgentComponent::active, "active", "Active", "", false);
		}

		REGISTER_COMPONENT(NavAgentComponent);
	};
}
