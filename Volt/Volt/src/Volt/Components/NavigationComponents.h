#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/AI/NavMesh/NavMeshAgent.h"

#include <Wire/Serialization.h>
#include <GEM/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct NavMeshComponent
	{
		PROPERTY(Name = NavMesh) AssetHandle handle = Asset::Null();

		CREATE_COMPONENT_GUID("{AA2F2165-219B-406F-8F0E-BB4E47F767DB}"_guid);
	}), NavMeshComponent);

	SERIALIZE_COMPONENT((struct NavMeshModifierComponent
	{
		PROPERTY(Name = Active) bool active = true;
		CREATE_COMPONENT_GUID("{4C7EB050-3DDA-43D5-A417-1B82DF260EC7}"_guid);
	}), NavMeshModifierComponent);

	SERIALIZE_COMPONENT((struct NavMeshBlockComponent
	{
		PROPERTY(Name = BlockTriangleArea) gem::vec3 area;
		PROPERTY(Name = Active) bool active = true;
		CREATE_COMPONENT_GUID("{695E6F7D-053F-4F0D-B350-8174E3828FC4}"_guid);
	}), NavMeshBlockComponent);

	SERIALIZE_COMPONENT((struct NavMeshAgentComponent
	{
		NavMeshAgent agent;
		CREATE_COMPONENT_GUID("{F29BA549-DD7D-407E-8024-6E281C4ED2AC}"_guid);
	}), NavMeshAgentComponent);
}
