#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/AI/NavMeshAgent.h"

#include <Wire/Serialization.h>
#include <GEM/gem.h>

namespace Volt
{
	SERIALIZE_COMPONENT((struct AgentComponent
	{
		NavMeshAgent agent;
		CREATE_COMPONENT_GUID("{F29BA549-DD7D-407E-8024-6E281C4ED2AC}"_guid);
	}), AgentComponent);
}
