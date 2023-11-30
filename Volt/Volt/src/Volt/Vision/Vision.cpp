#include "vtpch.h"
#include "Vision.h"

#include <Volt/Components/RenderingComponents.h>
#include <Volt/Components/CoreComponents.h>

#include <Volt/Utility/Noise.h>

#include "Volt/Events/MouseEvent.h"

Volt::Vision::Vision(Scene* aScene)
{
	myScene = aScene;
}

void Volt::Vision::OnEvent(Volt::Event& e)
{
	for (auto& cam : myVTCams)
	{
		cam.GetComponent<VisionCameraComponent>().OnEvent(e);
	}
}

void Volt::Vision::Initialize()
{
	std::vector<Volt::Entity> camEntIDs = myScene->GetAllEntitiesWith<Volt::VisionCameraComponent>();

	myVTCams.clear();

	for (auto ent : camEntIDs)
	{
		auto& camComp = ent.GetComponent<Volt::CameraComponent>();

		camComp.priority = 0;

		if (ent.GetComponent<Volt::VisionCameraComponent>().isDefault)
		{
			camComp.priority = 1;
			myActiveCamera = ent;
		}

		ent.GetComponent<Volt::VisionCameraComponent>().Init(ent);

		myVTCams.emplace_back(ent);
	}

	if (!myActiveCamera)
	{
		if (!myVTCams.empty())
		{
			myActiveCamera = myVTCams[0];
			myActiveCamera.GetComponent<Volt::CameraComponent>().priority = 1;
		}
	}

	if (myActiveCamera)
	{
		myTransitionCamera = CreateTransitionCamera();
	}
}

void Volt::Vision::Reset()
{
	myVTCams.clear();
	myScene->RemoveEntity(myTransitionCamera);
	myActiveCamera = Volt::Entity::Null();
}

void Volt::Vision::Update(float aDeltaTime)
{
	auto cams = myScene->GetAllEntitiesWith<Volt::VisionCameraComponent>();
	cams.erase(std::remove(cams.begin(), cams.end(), myTransitionCamera.GetID()), cams.end());

	if (myVTCams.size() != cams.size())
	{
		myVTCams.clear();
		for (auto& newCam : cams)
		{
			newCam.GetComponent<Volt::VisionCameraComponent>().Init(newCam);

			myVTCams.push_back(newCam);
		}
	}

	for (auto& camEnt : myVTCams)
	{
		camEnt.GetComponent<Volt::VisionCameraComponent>().Update(camEnt, aDeltaTime);
	}

	if (myIsBlending)
	{
		BlendCameras(aDeltaTime);
	}

	if (myIsShaking)
	{
		ShakeCamera(aDeltaTime);
	}
}

void Volt::Vision::InitTransitionCam()
{
	if (!myTransitionCamera)
	{
		myTransitionCamera = CreateTransitionCamera();
	}

	if (!myIsBlending)
	{
		if (myTriggerCamActive)
		{
			myTransitionCamera.SetPosition(myActiveTiggerCamera.GetPosition());
			myTransitionCamera.SetLocalRotation(myActiveTiggerCamera.GetRotation());
		}
		else
		{
			if (myLastActiveCamera)
			{
				myTransitionCamera.SetPosition(myLastActiveCamera.GetPosition());
				myTransitionCamera.SetLocalRotation(myLastActiveCamera.GetRotation());
			}
		}
		myTransitionCameraStartFoV = myActiveCamera.GetComponent<Volt::CameraComponent>().fieldOfView;
	}

	auto& vtCamComp = myTransitionCamera.GetComponent<Volt::VisionCameraComponent>();

	myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = 2;
	vtCamComp.blendType = myActiveCamera.GetComponent<Volt::VisionCameraComponent>().blendType;
	vtCamComp.additiveBlend = myActiveCamera.GetComponent<Volt::VisionCameraComponent>().additiveBlend;
	vtCamComp.myTargetFoV = myTransitionCameraStartFoV;

	myTransitionCamStartPos = myTransitionCamera.GetPosition();
	myTransitionCamStartRot = myTransitionCamera.GetLocalRotation();

	myLerpTime = myActiveCamera.GetComponent<Volt::VisionCameraComponent>().blendTime;
	myCurrentLerpTime = 0.f;

	myIsBlending = true;
}

void Volt::Vision::InitTransitionCam(const float& aBlendTime, const eBlendType blendType)
{
	if (!myTransitionCamera)
	{
		myTransitionCamera = CreateTransitionCamera();
	}

	if (!myIsBlending)
	{
		myTransitionCamera.SetPosition(myLastActiveCamera.GetPosition());
		myTransitionCamera.SetLocalRotation(myLastActiveCamera.GetRotation());
		myTransitionCameraStartFoV = myTransitionCamera.GetComponent<Volt::CameraComponent>().fieldOfView;
	}

	if (blendType == Volt::eBlendType::None || aBlendTime == 0)
	{
		myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = 0;
	}
	else
	{
		myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = 2;
	}

	myTransitionCamera.GetComponent<Volt::VisionCameraComponent>().blendType = blendType;
	myTransitionCamera.GetComponent<Volt::VisionCameraComponent>().additiveBlend = true;

	myTransitionCamStartPos = myTransitionCamera.GetPosition();
	myTransitionCamStartRot = myTransitionCamera.GetLocalRotation();

	myLerpTime = aBlendTime;
	myCurrentLerpTime = 0.f;

	myIsBlending = true;
}


Volt::Entity Volt::Vision::CreateTransitionCamera()
{
	Volt::Entity cam = myScene->CreateEntity();
	cam.GetComponent<Volt::TagComponent>().tag = "TransitionCamera";

	auto baseCamComp = myActiveCamera.GetComponent<Volt::CameraComponent>();

	auto& newBaseCamComp = cam.AddComponent<Volt::CameraComponent>();
	auto& vtCamComp = cam.AddComponent<Volt::VisionCameraComponent>();

	newBaseCamComp.fieldOfView = baseCamComp.fieldOfView;
	vtCamComp.myTargetFoV = 60;

	return cam;
}

void Volt::Vision::ShakeCamera(const float aDeltaTime)
{
	myNoiseStep += aDeltaTime;
	if (myNoiseStep > myCurrentShakeSetting.shakeTime)
	{
		myIsShaking = false;
		return;
	}
	glm::vec3 rotOffset;

	Volt::Noise::SetSeed((int32_t)myNoiseSeeds.x);
	Volt::Noise::SetFrequency(myCurrentShakeSetting.rotationAmount.x);
	rotOffset.x = Volt::Noise::GetNoise(myNoiseStep, 0, 0);

	Volt::Noise::SetSeed((int32_t)myNoiseSeeds.y);
	Volt::Noise::SetFrequency(myCurrentShakeSetting.rotationAmount.y);
	rotOffset.y = Volt::Noise::GetNoise(0, myNoiseStep, 0);

	Volt::Noise::SetSeed((int32_t)myNoiseSeeds.z);
	Volt::Noise::SetFrequency(myCurrentShakeSetting.rotationAmount.z);
	rotOffset.z = Volt::Noise::GetNoise(0, 0, myNoiseStep);

	float magPersentage = (myCurrentShakeSetting.shakeTime - myNoiseStep) / myCurrentShakeSetting.shakeTime;

	rotOffset *= glm::mix(0.f, myMaxShakeMag, magPersentage);

	myCurrentShakeCamera.SetRotation(myCurrentShakeCamera.GetRotation() * glm::quat(glm::radians(rotOffset)));
}

void Volt::Vision::BlendCameras(float aDeltaTime)
{
	auto& transitionBaseCamComp = myTransitionCamera.GetComponent<Volt::CameraComponent>();
	auto& activeBaseCamComp = myActiveCamera.GetComponent<Volt::CameraComponent>();

	auto& transitionVtCamComp = myTransitionCamera.GetComponent<Volt::VisionCameraComponent>();

	myCurrentLerpTime += aDeltaTime;

	if (myCurrentLerpTime >= myLerpTime)
	{
		transitionBaseCamComp.priority = 0;

		if (myTriggerCamActive)
		{
			myActiveTiggerCamera.GetComponent<Volt::CameraComponent>().priority = 1;
		}
		else
		{
			activeBaseCamComp.priority = 1;
		}

		myCurrentLerpTime = 0.f;
		myIsBlending = false;

		return;
	}

	float t = myCurrentLerpTime / myLerpTime;

	switch (transitionVtCamComp.blendType)
	{
		case Volt::eBlendType::None:
			t = 1.f;
			myCurrentLerpTime = myLerpTime;
			break;

		case Volt::eBlendType::EaseInAndOut:
			t = t * t * t * (t * (6.f * t - 15.f) + 10.f);
			break;

		case Volt::eBlendType::EaseIn:
			t = 1.f - glm::cos(t * glm::pi<float>() * 0.5f);
			break;

		case Volt::eBlendType::EaseOut:
			t = glm::sin(t * glm::pi<float>() * 0.5f);
			break;

		default:
			break;
	}

	if (transitionVtCamComp.additiveBlend == true)
	{
		myTransitionCamera.SetPosition(glm::mix(myLastActiveCamera.GetPosition(), myActiveCamera.GetPosition(), t));

		const auto q = glm::slerp(glm::normalize(myLastActiveCamera.GetRotation()), glm::normalize(myActiveCamera.GetRotation()), t);
		myTransitionCamera.SetRotation(q);
	}
	else
	{
		myTransitionCamera.SetPosition(glm::mix(myTransitionCamStartPos, myActiveCamera.GetPosition(), t));

		const auto q = glm::slerp(glm::normalize(myTransitionCamStartRot), glm::normalize(myActiveCamera.GetRotation()), t);
		myTransitionCamera.SetRotation(q);
	}

	transitionBaseCamComp.fieldOfView = glm::mix(myTransitionCameraStartFoV, activeBaseCamComp.fieldOfView, t);
}

void Volt::Vision::SetActiveCamera(const std::string aCamName)
{
	if (myActiveCamera.GetComponent<Volt::TagComponent>().tag == aCamName)
		return;

	for (auto cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 0;

		if (cam.GetComponent<Volt::TagComponent>().tag == aCamName)
		{
			SetLastActiveCamera(myActiveCamera);

			InitTransitionCam();
			myLastActiveCamera = myActiveCamera;
			myActiveCamera = cam;
		}
	}
}

void Volt::Vision::SetActiveCamera(const size_t aIndex)
{
	if (aIndex + 1 > myVTCams.size() || myActiveCamera == myVTCams[aIndex])
		return;

	for (auto cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 0;
	}

	SetLastActiveCamera(myActiveCamera);

	InitTransitionCam();

	myLastActiveCamera = myActiveCamera;
	myActiveCamera = myVTCams[aIndex];
}

void Volt::Vision::SetActiveCamera(const Volt::Entity aCamEntity)
{
	if (myActiveCamera == aCamEntity)
		return;

	for (auto& cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 0;

		if (aCamEntity == cam)
		{
			SetLastActiveCamera(myActiveCamera);
			myActiveCamera = cam;

			InitTransitionCam();
		}
	}
}

void Volt::Vision::SetActiveCamera(const EntityID aEntityID, const float& aBlendTime, eBlendType blendType)
{
	Volt::Entity camEnt = myScene->GetEntityFromUUID(aEntityID);


	if (camEnt)
	{
		if (myActiveCamera == camEnt)
			return;

		for (auto& cam : myVTCams)
		{
			cam.GetComponent<Volt::CameraComponent>().priority = 0;

			if (camEnt == cam)
			{
				SetLastActiveCamera(myActiveCamera);

				InitTransitionCam(aBlendTime, blendType);

				myLastActiveCamera = myActiveCamera;
				myActiveCamera = cam;
			}
		}
	}
}

void Volt::Vision::SetActiveCamera(const EntityID aEntityID)
{
	Volt::Entity ent = myScene->GetEntityFromUUID(aEntityID);

	if (ent)
	{
		SetActiveCamera(ent);
	}
}

const bool Volt::Vision::IsCameraActive(size_t aIndex)
{
	uint32_t prio = myVTCams[aIndex].GetComponent<Volt::CameraComponent>().priority;

	if (prio == 0)
	{
		return false;
	}
	return true;
}

void Volt::Vision::DoCameraShake(Volt::Entity aCameraEntity, const CameraShakeSettings& aShakeSetting)
{
	myCurrentShakeCamera = aCameraEntity;
	myCurrentShakeSetting = aShakeSetting;
	myIsShaking = true;
	myNoiseStep = 0.f;
	myMaxShakeMag = myCurrentShakeSetting.rotationMagnitude;
	myCurrentShakeMag = myMaxShakeMag;

	myNoiseSeeds = { Volt::Noise::GetRandomSeed(),Volt::Noise::GetRandomSeed(), Volt::Noise::GetRandomSeed() };
}

void Volt::Vision::SetCameraFollow(Volt::Entity aCamera, Volt::Entity aFollowEntity)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().followId = aFollowEntity.GetID();
		}
	}
}

void Volt::Vision::SetCameraLookAt(Volt::Entity aCamera, Volt::Entity aLookAtEnt)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().lookAtId = aLookAtEnt.GetID();
		}
	}
}

void Volt::Vision::SetCameraFocusPoint(Volt::Entity aCamera, Volt::Entity aFocusEntity)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().collisionRayPoint = aFocusEntity.GetID();
		}
	}
}

void Volt::Vision::SetCameraDampAmount(Volt::Entity aCamera, float aDampAmount)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().damping = aDampAmount;
		}
	}
}

void Volt::Vision::SetCameraFieldOfView(Volt::Entity aCamera, float aFoV)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().myTargetFoV = aFoV;
		}
	}
}

void Volt::Vision::SetCameraLocked(Volt::Entity aCamera, bool locked)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().myIsLocked = locked;
		}
	}
}

void Volt::Vision::SetCameraMouseSensentivity(Volt::Entity aCamera, float mouseSens)
{
	if (myVTCams.empty()) { return; }

	if (aCamera)
	{
		if (aCamera.HasComponent<Volt::VisionCameraComponent>())
		{
			aCamera.GetComponent<Volt::VisionCameraComponent>().mouseSensitivity = mouseSens;
		}
	}
}

const std::vector<Volt::Entity> Volt::Vision::GetAllCamerasInScene()
{
	return myVTCams;
}

void Volt::Vision::SetTriggerCamera(const Volt::Entity aTriggerCamera)
{
	for (auto& cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 0;

		if (aTriggerCamera == cam)
		{
			InitTransitionCam();
			myActiveTiggerCamera = cam;
			myActiveCamera = myActiveTiggerCamera;
		}
	}
}
