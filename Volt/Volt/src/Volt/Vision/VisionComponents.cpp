#include "vtpch.h"
#include "VisionComponents.h"

#include <Volt/Components/Components.h>

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Physics/PhysicsScene.h"

#include "Volt/Events/MouseEvent.h"
#include "Volt/Events/GameEvent.h"

#include "Volt/Input/Input.h"

void Volt::VisionCameraComponent::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::MouseMovedEvent>([this](Volt::MouseMovedEvent& e)
	{
		myMousePos = { e.GetX(), e.GetY() };
		return false;
	});

	dispatcher.Dispatch<Volt::OnGameStateChangedEvent>([this](Volt::OnGameStateChangedEvent& e)
	{
		if (e.GetState() == Volt::OnGameStateChangedEvent::PLAY)
		{
			auto [x, y] = Volt::Input::GetMousePosition();

			myMousePos.x = x;
			myMousePos.y = y;

			myLastMousePos.x = x;
			myLastMousePos.y = y;
		}

		return false;
	});
}

void Volt::VisionCameraComponent::Init(Entity& camEntity)
{
	if (cameraType == Volt::eCameraType::ThirdPerson || cameraType == Volt::eCameraType::FirstPerson)
	{
		myMaxFocalDist = focalDistance;
		myCurrentFocalDist = myMaxFocalDist;
		myCurrentLerpTime = myLerpTime;

		auto [x, y] = Volt::Input::GetMousePosition();

		myMousePos.x = x;
		myMousePos.y = y;

		myLastMousePos.x = x;
		myLastMousePos.y = y;

		if (damping > 0)
		{
			Volt::Entity target = Volt::Entity{ followId, camEntity.GetScene() };

			if (target)
			{
				myDampedFocalPoint = target.GetPosition() + camEntity.GetRight() * offset.x + camEntity.GetUp() * offset.y + camEntity.GetForward() * offset.z;
			}
		}
	}
	myTargetFoV = camEntity.GetComponent<CameraComponent>().fieldOfView;
}

void Volt::VisionCameraComponent::Update(Entity& camEntity, float aDeltaTime)
{
	switch (cameraType)
	{
		case Volt::eCameraType::Free:
			FreeController(camEntity, aDeltaTime);
			break;
		case Volt::eCameraType::FirstPerson:
			FPSController(camEntity, aDeltaTime);
			break;
		case Volt::eCameraType::ThirdPerson:
			TPSController(camEntity, aDeltaTime);
			break;
		default:
			break;
	}

	auto& baseCamComp = camEntity.GetComponent<CameraComponent>();
	
	baseCamComp.fieldOfView = gem::lerp(baseCamComp.fieldOfView, myTargetFoV, 1.f - gem::pow(2.f, -10.f * aDeltaTime));
}

void Volt::VisionCameraComponent::FreeController(Entity& camEntity, float aDeltaTime)
{
	if (followId != 0)
	{
		Volt::Entity followEnt = Volt::Entity{ followId, camEntity.GetScene() };

		if (followEnt)
		{
			gem::vec3 targetPos = followEnt.GetPosition() + offset;
			const gem::vec3 camPos = camEntity.GetPosition();

			if (xFollowLock) targetPos.x = camPos.x;
			if (yFollowLock) targetPos.y = camPos.y;
			if (zFollowLock) targetPos.z = camPos.z;

			if (damping > 0)
			{
				camEntity.SetPosition(gem::lerp(camPos, targetPos, 1 - gem::pow(2.f, -aDeltaTime * damping)));
			}
			else
			{
				camEntity.SetPosition(targetPos);
			}
		}
	}

	if (lookAtId != 0)
	{
		Entity lookAtEnt = Entity{ lookAtId, camEntity.GetScene() };

		if (lookAtEnt)
		{
			gem::vec3 lookAtPos = lookAtEnt.GetPosition();
			camEntity.SetRotation(gem::quatLookAtLH(gem::normalize(lookAtPos - camEntity.GetPosition()), { 0,1,0 }));
		}
	}
}

void Volt::VisionCameraComponent::FPSController(Entity& camEntity, float aDeltaTime)
{
	Volt::Entity target = Volt::Entity{ followId, camEntity.GetScene() };

	if (!target) { return; }

	const gem::vec2 deltaPos = (myMousePos - myLastMousePos);

	myLastMousePos = myMousePos;

	const float yawSign = camEntity.GetUp().y < 0.f ? -1.f : 1.f;
	myYawDelta = yawSign * deltaPos.x * mouseSensitivity;
	myPitchDelta = deltaPos.y * mouseSensitivity;

	myRotation += gem::radians(gem::vec3{ myPitchDelta, myYawDelta, 0.f });

	myRotation.x = gem::clamp(myRotation.x, -1.35f, 1.35f);

	if (myIsLocked == false)
	{
		camEntity.SetLocalRotation(gem::quat(myRotation));
	}
}


void Volt::VisionCameraComponent::TPSController(Entity& camEntity, float aDeltaTime)
{
	const gem::vec2 deltaPos = (myMousePos - myLastMousePos);

	myLastMousePos = myMousePos;

	const float yawSign = camEntity.GetUp().y < 0.f ? -1.f : 1.f;
	myYawDelta = yawSign * deltaPos.x * mouseSensitivity;
	myPitchDelta = deltaPos.y * mouseSensitivity;

	myRotation += gem::radians(gem::vec3{ myPitchDelta, myYawDelta, 0.f });

	myRotation.x = gem::clamp(myRotation.x, -1.35f, 1.35f);

	Volt::Entity target = Volt::Entity{ followId, camEntity.GetScene() };

	if (target)
	{
		if (myIsLocked == false)
		{
			camEntity.SetLocalRotation(gem::quat(myRotation));
		}
		gem::vec3 focalTargetPoint = target.GetPosition() + camEntity.GetRight() * offset.x + target.GetUp() * offset.y + camEntity.GetForward() * offset.z;

		const gem::vec3 dir = camEntity.GetPosition() - focalTargetPoint;
		gem::vec3 position = { 0 };

		Volt::Entity rayFocalPointEnt = Volt::Entity{ collisionRayPoint, camEntity.GetScene() };

		if (isColliding)
		{
			//Temp Vars
			Volt::RaycastHit hit;

			gem::vec3 rayDir;
			gem::vec3 rayFromPos;
			gem::vec3 rayTargetPoint = focalTargetPoint;

			if (rayFocalPointEnt)
			{
				rayTargetPoint = rayFocalPointEnt.GetPosition() + camEntity.GetRight() * offset.x + rayFocalPointEnt.GetUp() * offset.y + camEntity.GetForward() * offset.z;
			}

			const gem::vec3 dirFromTargetToOffset = rayTargetPoint - target.GetPosition();

			//Check if any layer masks
			uint32_t mask = 0;
			if (!layerMasks.empty())
			{
				for (int i = 0; i < layerMasks.size(); i++)
				{
					mask |= 1 << layerMasks[i];
				}
			}

			//Raycast from target to offset to know if offset is inside wall and has to be moved
			if (rayFocalPointEnt)
			{
				if (Volt::Physics::GetScene()->Raycast(rayFocalPointEnt.GetPosition(), gem::normalize(dirFromTargetToOffset), gem::length(dirFromTargetToOffset) + 25, &hit, mask))
				{
					rayTargetPoint = rayFocalPointEnt.GetPosition() + (gem::normalize(dirFromTargetToOffset) * hit.distance) - 25;
					focalTargetPoint = rayTargetPoint;
				}
			}
			else
			{
				if (Volt::Physics::GetScene()->Raycast(target.GetPosition(), gem::normalize(dirFromTargetToOffset), gem::length(dirFromTargetToOffset) + 25, &hit, mask))
				{
					rayTargetPoint = target.GetPosition() + (gem::normalize(dirFromTargetToOffset) * hit.distance) - 25;
					focalTargetPoint = rayTargetPoint;
				}
			}


			rayDir = camEntity.GetPosition() - rayTargetPoint;
			rayFromPos = rayTargetPoint;

			bool rayHit = false;

			const gem::vec3 normRayDir = gem::normalize(rayDir);

			if (mask == 0)
			{
				rayHit = Volt::Physics::GetScene()->Raycast(rayFromPos, normRayDir, myMaxFocalDist + collisionRadius, &hit);
			}
			else
			{
				rayHit = Volt::Physics::GetScene()->Raycast(rayFromPos, normRayDir, myMaxFocalDist + collisionRadius, &hit, mask);
			}

			if (rayHit)
			{
				myCurrentLerpTime = 0.f;
				myCurrentFocalDist = gem::length(normRayDir * hit.distance) - collisionRadius;
				myLastHitFocalDist = myCurrentFocalDist;
			}
			else
			{
				//If camera is not colliding it lerps back to place
				myCurrentLerpTime += aDeltaTime;
				if (myCurrentLerpTime >= myLerpTime)
				{
					myCurrentLerpTime = myLerpTime;
				}

				float t = myCurrentLerpTime / myLerpTime;

				t = gem::sin(t * gem::pi() * 0.5f); //Ease Ou
				myCurrentFocalDist = gem::lerp(myLastHitFocalDist, myMaxFocalDist, t);
			}
		}

		if (damping > 0)
		{
			myDampedFocalPoint = gem::lerp(myDampedFocalPoint, focalTargetPoint, 1 - gem::pow(2.f, -damping * aDeltaTime));

			if (!xShouldDamp) { myDampedFocalPoint.x = focalTargetPoint.x; }
			if (!yShouldDamp) { myDampedFocalPoint.y = focalTargetPoint.y; }
			if (!zShouldDamp) { myDampedFocalPoint.z = focalTargetPoint.z; }

			position = myDampedFocalPoint - camEntity.GetForward() * myCurrentFocalDist;
		}
		else
		{
			position = focalTargetPoint - camEntity.GetForward() * myCurrentFocalDist;
		}

		camEntity.SetPosition(position);
	}

}