#pragma once

#include "Volt/Asset/Asset.h"

#include <Wire/Serialization.h>
#include <gem/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct NavMeshComponent
	{
		CREATE_COMPONENT_GUID("{6643C885-4A12-4527-8AA7-7894A730BAC5}"_guid);
	}), NavMeshComponent);

	SERIALIZE_COMPONENT((struct NavLinkComponent
	{
		PROPERTY(Name = Start, Visible = true) gem::vec3 start = { 0.f };
		PROPERTY(Name = End, Visible = true) gem::vec3 end = { 0.f };
		PROPERTY(Name = Bidirectional, Visible = true) bool bidirectional = true;
		PROPERTY(Name = Active, Visible = true) bool active = true;

		CREATE_COMPONENT_GUID("{7104A8A0-3657-4840-B172-89F5E79E64A6}"_guid);
	}), NavLinkComponent);

	SERIALIZE_ENUM((enum class ObstacleAvoidanceQuality : uint32_t
	{
		None = 0,
		Low,
		Medium,
		High
	}), ObstacleAvoidanceQuality);

	SERIALIZE_COMPONENT((struct NavAgentComponent
	{
		PROPERTY(Name = Radius, Visible = true) float radius = 60.f;
		PROPERTY(Name = Height, Visible = true) float height = 200.f;
		PROPERTY(Name = MaxSpeed, Visible = true) float maxSpeed = 300.f;
		PROPERTY(Name = Acceleration, Visible = true) float acceleration = 1000.f;
		PROPERTY(Name = Separation Weight, Visible = true) float separationWeight = 0.f;
		PROPERTY(Name = Obstacle Avoidance Quality, Visible = true, SpecialType = Enum) ObstacleAvoidanceQuality obstacleAvoidanceQuality = ObstacleAvoidanceQuality::None;
		PROPERTY(Name = Active, Visible = true) bool active = true;

		CREATE_COMPONENT_GUID("{2B4469CE-9B15-4FA9-ABA6-77BA83465357}"_guid);
	}), NavAgentComponent);
}
