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

	SERIALIZE_ENUM((enum class eBlendType : uint32_t
	{
		None,
		EaseIn,
		EaseOut,
		EaseInAndOut

	}), eBlendType)

	SERIALIZE_COMPONENT((struct DirectorComponent
	{
		PROPERTY(Name = Active Camera, Visible = true) Wire::EntityId activeCam = Wire::NullID;

		CREATE_COMPONENT_GUID("{754A371B-A544-4A55-AE6A-CE6CA946F6D9}"_guid)
	}), DirectorComponent);

	SERIALIZE_COMPONENT((struct TestControllerComponent
	{
		PROPERTY(Name = Aim Camera, Visible = true) Wire::EntityId AimCamera = Wire::NullID;

		CREATE_COMPONENT_GUID("{4EA4F64B-3522-4ECC-9496-6E2CFE61D421}"_guid)
	}), TestControllerComponent);

	SERIALIZE_COMPONENT((struct VTCamComponent
	{
		PROPERTY(Name = Blend Time, Visible = false) float blendTime = 1.f;
		PROPERTY(Name = FOV, Visible = false) float fov = 60.f;

		PROPERTY(Name = Camera Type, SpecialType = Enum, Visible = false) eCameraType cameraType = eCameraType::Default;
		PROPERTY(Name = Blend Type, SpecialType = Enum, Visible = false) eBlendType blendType = eBlendType::None;

		PROPERTY(Name = Damping, Visible = false) float damping = 0.f;
		PROPERTY(Name = Offset, Visible = false) gem::vec3 offset = { 0 };

		PROPERTY(Name = Follow, Visible = false) Wire::EntityId followId = Wire::NullID;
		PROPERTY(Name = LookAt, Visible = false) Wire::EntityId lookAtId = Wire::NullID;

		PROPERTY(Name = Focal Distance, Visible = false) float focalDistance = 1000.f;
		PROPERTY(Name = Mouse Sensitivity, Visible = false) float mouseSensitivity = 0.15f;

		PROPERTY(Name = Is Default, Visible = false) bool isDefault = false;

		CREATE_COMPONENT_GUID("{2C5E4323-6F22-4AFD-BE83-9DC419B8C45F}"_guid)
	}), VTCamComponent);
}