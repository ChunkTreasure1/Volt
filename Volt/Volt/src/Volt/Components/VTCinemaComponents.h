#pragma once
#include <Wire/Serialization.h>
#include <gem/gem.h>
#include <string>

namespace Volt
{
	SERIALIZE_ENUM((enum class eCameraType : uint32_t
	{
		Default,
		Free,
		ThirdPerson,
		FirstPerson

	}), eCameraType)

	SERIALIZE_COMPONENT((struct DirectorComponent
	{
		PROPERTY(Name = Active Camera, Visible = true) Wire::EntityId activeCam = Wire::NullID;

		CREATE_COMPONENT_GUID("{754A371B-A544-4A55-AE6A-CE6CA946F6D9}"_guid)
	}), DirectorComponent);

	SERIALIZE_COMPONENT((struct VTCamComponent
	{
		PROPERTY(Name = Is Default, Visible = false) bool isDefault = false;

		PROPERTY(Name = Camera Type, SpecialType = Enum, Visible = false) eCameraType cameraType = eCameraType::Default;

		PROPERTY(Name = FOV, Visible = false) float fov = 60.f;
		PROPERTY(Name = Damping, Visible = false) float damping = 0.f;
		PROPERTY(Name = Offset, Visible = false) gem::vec3 offset = { 0 };

		PROPERTY(Name = Follow, Visible = false) Wire::EntityId followId = Wire::NullID;
		PROPERTY(Name = LookAt, Visible = false) Wire::EntityId lookAtId = Wire::NullID;

		PROPERTY(Name = Focal Distance, Visible = false) float focalDistance = 0.f;
		PROPERTY(Name = Mouse Sensitivity, Visible = false) float mouseSensitivity = 0.f;

		CREATE_COMPONENT_GUID("{2C5E4323-6F22-4AFD-BE83-9DC419B8C45F}"_guid)
	}), VTCamComponent);
}