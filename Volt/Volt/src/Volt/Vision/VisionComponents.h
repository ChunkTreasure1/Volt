#pragma once
#include <Wire/Serialization.h>
#include <glm/glm.hpp>
#include <string>

namespace Volt
{
	class Event;
	class Scene;
	class Entity;

	SERIALIZE_ENUM((enum class eCameraType : uint32_t
	{
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

		SERIALIZE_COMPONENT((struct VisionTriggerComponent
	{
		PROPERTY(Name = Camera, Visible = true) Wire::EntityId triggerCam = Wire::NullID;
		PROPERTY(Name = Force Camera, Visible = true) bool forceActiveCam = false;

		CREATE_COMPONENT_GUID("{E846C1DE-F199-4BFF-9D15-9E73B8D1C941}"_guid)
	}), VisionTriggerComponent);

	SERIALIZE_COMPONENT((struct VisionCameraComponent
	{
		PROPERTY(Name = Blend Time, Visible = false) float blendTime = 1.f;
		PROPERTY(Name = Field Of View, Visible = false) float fieldOfView = 60.f;

		PROPERTY(Name = Ignored Layers, Visible = false) std::vector<uint32_t> layerMasks = std::vector<uint32_t>();

		PROPERTY(Name = Camera Type, SpecialType = Enum, Visible = false) eCameraType cameraType = eCameraType::Free;
		PROPERTY(Name = Blend Type, SpecialType = Enum, Visible = false) eBlendType blendType = eBlendType::None;

		PROPERTY(Name = Damping, Visible = false) float damping = 0.f;
		PROPERTY(Name = Offset, Visible = false) glm::vec3 offset = { 0 };

		PROPERTY(Name = Follow, Visible = false) Wire::EntityId followId = Wire::NullID;
		PROPERTY(Name = LookAt, Visible = false) Wire::EntityId lookAtId = Wire::NullID;
		PROPERTY(Name = Collision Focus Point, Visible = false) Wire::EntityId collisionRayPoint = Wire::NullID;

		PROPERTY(Name = Focal Distance, Visible = false) float focalDistance = 1000.f;
		PROPERTY(Name = Mouse Sensitivity, Visible = false) float mouseSensitivity = 0.15f;

		PROPERTY(Name = Collision Sphere Radius, Visible = false) float collisionRadius = 100.f;

		PROPERTY(Name = Collision, Visible = false) bool isColliding = false;
		PROPERTY(Name = Is Default, Visible = false) bool isDefault = false;

		PROPERTY(Name = X FollowLock, Visible = false) bool xFollowLock = false;
		PROPERTY(Name = Y FollowLock, Visible = false) bool yFollowLock = false;
		PROPERTY(Name = Z FollowLock, Visible = false) bool zFollowLock = false;

		PROPERTY(Name = X ShouldDamp, Visible = false) bool xShouldDamp = true;
		PROPERTY(Name = Y ShouldDamp, Visible = false) bool yShouldDamp = true;
		PROPERTY(Name = Z ShouldDamp, Visible = false) bool zShouldDamp = true;

		PROPERTY(Name = Additive Blend, Visible = false) bool additiveBlend = false;

		void Init(Entity& camEntity);
		void Update(Entity& camEntity, float aDeltaTime);

		void OnEvent(Volt::Event& e);

		void FreeController(Entity& camEntity, float aDeltaTime);
		void TPSController(Entity& camEntity, float aDeltaTime);
		void FPSController(Entity& camEntity, float aDeltaTime);

		CREATE_COMPONENT_GUID("{1213269A-DB9A-4BC2-82DF-6656A41451A3}"_guid)
		float myTargetFoV = 0.f;
		bool myIsLocked = false;

	private:
		float myYawDelta = { 0 };
		float myPitchDelta = { 0 };
		float myMaxFocalDist = 0.f;
		float myCurrentFocalDist = 0.f;
		float myLastHitFocalDist = 0.f;
		float myLerpTime = 0.5f;
		float myCurrentLerpTime = 0.f;
		float myCurrentDampTime = 0.f;

		glm::vec3 myRotation = { 0 };
		glm::vec3 myPosition = { 0 };

		glm::vec3 myDampedFocalPoint = { 0 };

		glm::vec2 myLastMousePos = { 0 };
		glm::vec2 myMousePos = { 0.f };


	}), VisionCameraComponent);
}
