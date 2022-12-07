#pragma once
#include <Wire/Serialization.h>
#include <gem/gem.h>
#include <string>

namespace Volt
{
	SERIALIZE_ENUM((enum class eCameraType : uint32_t
	{
		Free,
		ThirdPerson,
		FirstPerson

	}), eCameraType)

	SERIALIZE_COMPONENT((struct DirectorComponent
	{
		PROPERTY(Name = Is Active) bool active = true;

		CREATE_COMPONENT_GUID("{754A371B-A544-4A55-AE6A-CE6CA946F6D9}"_guid)
	}), DirectorComponent);

	SERIALIZE_COMPONENT((struct VTCamComponent
	{
		PROPERTY(Name = Camera Type, SpecialType = Enum, Visible = false) eCameraType cameraType = eCameraType::Free;
		PROPERTY(Name = FOV, Visible = false) float fov = 60.f;
		PROPERTY(Name = Offset, Visible = false) gem::vec3 offset = { 0 };
		PROPERTY(Name = Target, Visible = false) Wire::EntityId targetId = Wire::NullID;
		PROPERTY(Name = Look At, Visible = false) Wire::EntityId lookAtId = Wire::NullID;

		CREATE_COMPONENT_GUID("{2C5E4323-6F22-4AFD-BE83-9DC419B8C45F}"_guid)
	}), VTCamComponent);
}