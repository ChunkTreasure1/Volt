#include "vtpch.h"
#include "Vision.h"

#include <Volt/Components/Components.h>

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
	std::vector<Wire::EntityId> camEntIDs = myScene->GetAllEntitiesWith<Volt::VisionCameraComponent>();

	myVTCams.clear();

	for (auto ID : camEntIDs)
	{
		Volt::Entity ent = { ID, myScene };

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
	myActiveCamera = Volt::Entity{ 0,nullptr };
}

void Volt::Vision::Update(float aDeltaTime)
{
	std::vector<Wire::EntityId> cams = myScene->GetRegistry().GetComponentView<Volt::VisionCameraComponent>();

	if (myVTCams.size() != cams.size())
	{
		myVTCams.clear();
		for (auto& cam : cams)
		{
			Volt::Entity newCam{ cam, myScene };

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
		myTransitionCameraStartFoV = myTransitionCamera.GetComponent<Volt::CameraComponent>().fieldOfView;
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
		myTransitionCamera.GetComponent<Volt::CameraComponent>().priority = -1;
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
	gem::vec3 rotOffset;

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

	rotOffset *= gem::lerp(0.f, myMaxShakeMag, magPersentage);

	myCurrentShakeCamera.SetRotation(myCurrentShakeCamera.GetRotation() * gem::quat(gem::radians(rotOffset)));
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
			t = 1.f - gem::cos(t * gem::pi() * 0.5f);
			break;

		case Volt::eBlendType::EaseOut:
			t = gem::sin(t * gem::pi() * 0.5f);
			break;

		default:
			break;
	}

	if (transitionVtCamComp.additiveBlend == true)
	{
		myTransitionCamera.SetPosition(gem::lerp(myLastActiveCamera.GetPosition(), myActiveCamera.GetPosition(), t));

		const auto q = gem::slerp(gem::normalize(myLastActiveCamera.GetRotation()), gem::normalize(myActiveCamera.GetRotation()), t);
		myTransitionCamera.SetRotation(q);
	}
	else
	{
		myTransitionCamera.SetPosition(gem::lerp(myTransitionCamStartPos, myActiveCamera.GetPosition(), t));

		const auto q = gem::slerp(gem::normalize(myTransitionCamStartRot), gem::normalize(myActiveCamera.GetRotation()), t);
		myTransitionCamera.SetRotation(q);
	}

	transitionBaseCamComp.fieldOfView = gem::lerp(myTransitionCameraStartFoV, activeBaseCamComp.fieldOfView, t);
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

	for (const auto& cam : myVTCams)
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

void Volt::Vision::SetActiveCamera(const Wire::EntityId aEntityID, const float& aBlendTime, eBlendType blendType)
{
	Volt::Entity camEnt = Volt::Entity{ aEntityID, myScene };

	if (camEnt)
	{
		if (myActiveCamera == camEnt)
			return;

		for (const auto& cam : myVTCams)
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

void Volt::Vision::SetActiveCamera(const Wire::EntityId aEntityID)
{
	Volt::Entity ent = Volt::Entity{ aEntityID, myScene };

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
			aCamera.GetComponent<Volt::VisionCameraComponent>().followId = aFollowEntity.GetId();
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
			aCamera.GetComponent<Volt::VisionCameraComponent>().lookAtId = aLookAtEnt.GetId();
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
			aCamera.GetComponent<Volt::VisionCameraComponent>().collisionRayPoint = aFocusEntity.GetId();
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

const std::vector<Volt::Entity> Volt::Vision::GetAllCamerasInScene()
{
	return myVTCams;
}

void Volt::Vision::SetTriggerCamera(const Volt::Entity aTriggerCamera)
{
	for (const auto& cam : myVTCams)
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