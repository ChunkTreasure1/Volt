#pragma once

#include <Wire/Serialization.h>
#include <gem/gem.h>

SERIALIZE_COMPONENT((struct WandererComponent
{
	CREATE_COMPONENT_GUID("{282FA5FB-6A77-47DB-8340-3DEEF1A1FBBD}"_guid);
}), WandererComponent);