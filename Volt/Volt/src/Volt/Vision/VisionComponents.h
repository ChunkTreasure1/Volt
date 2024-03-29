#pragma once
#include "Volt/Scene/Reflection/ComponentRegistry.h"
#include "Volt/Scene/Reflection/ComponentReflection.h"

#include "Volt/Scene/Entity.h"

#include <glm/glm.hpp>
#include <string>

namespace Volt
{
	class Event;
	class Scene;
	class Entity;

	enum class eCameraType : uint32_t
	{
		Free,
		ThirdPerson,
		FirstPerson
	};

	inline static void ReflectType(TypeDesc<eCameraType>& reflect)
	{
		reflect.SetGUID("{3B9E92F4-0B5B-48CF-ADFC-AFE72F871208}"_guid);
		reflect.SetLabel("Camera Type");
		reflect.AddConstant(eCameraType::Free, "free", "Free");
		reflect.AddConstant(eCameraType::ThirdPerson, "thirdPerson", "Third Person");
		reflect.AddConstant(eCameraType::FirstPerson, "firstPerson", "First Person");
	}

	enum class eBlendType : uint32_t
	{
		None,
		EaseIn,
		EaseOut,
		EaseInAndOut
	};

	inline static void ReflectType(TypeDesc<eBlendType>& reflect)
	{
		reflect.SetGUID("{7E7EB035-2857-47E3-A4DF-9BE639D1A10F}"_guid);
		reflect.SetLabel("Blend Type");
		reflect.AddConstant(eBlendType::None, "none", "None");
		reflect.AddConstant(eBlendType::EaseIn, "easeIn", "Ease In");
		reflect.AddConstant(eBlendType::EaseOut, "easeOut", "Ease Out");
		reflect.AddConstant(eBlendType::EaseInAndOut, "easeInAndOut", "Ease In And Out");
	}

	struct VisionTriggerComponent
	{
		EntityID triggerCam = Entity::NullID();
		bool forceActiveCam = false;

		static void ReflectType(TypeDesc<VisionTriggerComponent>& reflect)
		{
			reflect.SetGUID("{E846C1DE-F199-4BFF-9D15-9E73B8D1C941}"_guid);
			reflect.SetLabel("Vision Trigger Component");
			reflect.AddMember(&VisionTriggerComponent::triggerCam, "triggerCam", "Trigger Camera", "", Entity::NullID());
			reflect.AddMember(&VisionTriggerComponent::forceActiveCam, "forceActiveCam", "Force Active Camera", "", false);
		}

		REGISTER_COMPONENT(VisionTriggerComponent);
	};

	struct VisionCameraComponent
	{
		float blendTime = 1.f;
		float fieldOfView = 60.f;
		std::vector<uint32_t> layerMasks = std::vector<uint32_t>();
		eCameraType cameraType = eCameraType::Free;
		eBlendType blendType = eBlendType::None;
		float damping = 0.f;
		glm::vec3 offset = { 0 };
		EntityID followId = Entity::NullID();
		EntityID lookAtId = Entity::NullID();
		EntityID collisionRayPoint = Entity::NullID();
		float focalDistance = 1000.f;
		float mouseSensitivity = 0.15f;
		float collisionRadius = 100.f;
		bool isColliding = false;
		bool isDefault = false;
		bool xFollowLock = false;
		bool yFollowLock = false;
		bool zFollowLock = false;
		bool xShouldDamp = true;
		bool yShouldDamp = true;
		bool zShouldDamp = true;
		bool additiveBlend = false;

		static void ReflectType(TypeDesc<VisionCameraComponent>& reflect)
		{
			reflect.SetGUID("{1213269A-DB9A-4BC2-82DF-6656A41451A3}"_guid);
			reflect.SetLabel("Vision Camera Component");
			reflect.AddMember(&VisionCameraComponent::blendTime, "blendTime", "Blend Time", "", 1.f);
			reflect.AddMember(&VisionCameraComponent::fieldOfView, "fieldOfView", "Field Of View", "", 60.f);
			reflect.AddMember(&VisionCameraComponent::layerMasks, "layerMasks", "Layer Masks", "", std::vector<uint32_t>());
			reflect.AddMember(&VisionCameraComponent::cameraType, "cameraType", "Camera Type", "", eCameraType::Free);
			reflect.AddMember(&VisionCameraComponent::blendType, "blendType", "Blend Type", "", eBlendType::None);
			reflect.AddMember(&VisionCameraComponent::damping, "damping", "Damping", "", 0.f);
			reflect.AddMember(&VisionCameraComponent::offset, "offset", "Offset", "", glm::vec3{ 0.f });
			reflect.AddMember(&VisionCameraComponent::followId, "followId", "Follow ID", "", Entity::NullID());
			reflect.AddMember(&VisionCameraComponent::lookAtId, "lookAtId", "Look At ID", "", Entity::NullID());
			reflect.AddMember(&VisionCameraComponent::collisionRayPoint, "collisionRayPoint", "Collision Ray Point", "", Entity::NullID());
			reflect.AddMember(&VisionCameraComponent::focalDistance, "focalDistance", "Focal Distance", "", 1000.f);
			reflect.AddMember(&VisionCameraComponent::mouseSensitivity, "mouseSensitivity", "Mouse Sensitivity", "", 0.15f);
			reflect.AddMember(&VisionCameraComponent::collisionRadius, "collisionRadius", "Collision Radius", "", 100.f);
			reflect.AddMember(&VisionCameraComponent::isColliding, "isColliding", "Is Colliding", "", false);
			reflect.AddMember(&VisionCameraComponent::isDefault, "isDefault", "Is Default", "", false);
			reflect.AddMember(&VisionCameraComponent::xFollowLock, "xFollowLock", "X Follow Lock", "", false);
			reflect.AddMember(&VisionCameraComponent::yFollowLock, "yFollowLock", "Y Follow Lock", "", false);
			reflect.AddMember(&VisionCameraComponent::zFollowLock, "zFollowLock", "Z Follow Lock", "", false);
			reflect.AddMember(&VisionCameraComponent::xShouldDamp, "xShouldDamp", "X Should Damp", "", true);
			reflect.AddMember(&VisionCameraComponent::yShouldDamp, "yShouldDamp", "Y Should Damp", "", true);
			reflect.AddMember(&VisionCameraComponent::zShouldDamp, "zShouldDamp", "Z Should Damp", "", true);
			reflect.AddMember(&VisionCameraComponent::additiveBlend, "additiveBlend", "Additive Blend", "", false);
		}

		void Init(Entity& camEntity);
		void Update(Entity& camEntity, float aDeltaTime);

		void OnEvent(Volt::Event& e);

		void FreeController(Entity& camEntity, float aDeltaTime);
		void TPSController(Entity& camEntity, float aDeltaTime);
		void FPSController(Entity& camEntity, float aDeltaTime);

		float myTargetFoV = 0.f;
		bool myIsLocked = false;

		REGISTER_COMPONENT(VisionCameraComponent);

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


	};
}
