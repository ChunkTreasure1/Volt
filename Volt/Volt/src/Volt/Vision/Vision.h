#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Vision/VisionComponents.h"

namespace Volt
{
	class Event;
	class Scene;

	struct CameraShakeSettings
	{
		gem::vec3 rotationAmount = { 1.f,1.f,1.f };
		gem::vec3 translationAmount = { 0.f,0.f,0.f };

		float rotationMagnitude = 1.f;
		float translationMagnitude = 1.f;
		float shakeTime = 1.f;
	};

	class Vision
	{
	public:
		Vision(Scene* aScene);
		~Vision() = default;

		void Initialize();
		void Reset(); // For Timeline

		void Update(float aDeltaTime);

		void OnEvent(Volt::Event& e);

		void SetActiveCamera(const std::string aCamName);
		void SetActiveCamera(const size_t aIndex);
		void SetActiveCamera(const Volt::Entity aCamEntity);
		void SetActiveCamera(const Wire::EntityId aEntityID);
		void SetActiveCamera(const Wire::EntityId aEntityID, const float& aBlendTime, Volt::eBlendType blendType);

		const Volt::Entity GetActiveCamera() { return !myActiveCamera.IsNull() ? myActiveCamera : Volt::Entity{ 0,nullptr }; }
		const Volt::Entity GetTriggerCamera() { return !myActiveTiggerCamera.IsNull() ? myActiveTiggerCamera : Volt::Entity{ 0,nullptr }; }
		const Volt::Entity GetLastActiveCamera() { return !myLastActiveCamera.IsNull() ? myLastActiveCamera : Volt::Entity{ 0,nullptr }; }
		const std::vector<Volt::Entity> GetAllCamerasInScene();

		void SetLastActiveCamera(Volt::Entity aCamEnt) { myLastActiveCamera = aCamEnt; };

		void SetTriggerCamActive(bool active) { myTriggerCamActive = active; }
		void SetTriggerCamera(const Volt::Entity aTriggerCamera);
		bool IsTriggerCamActive() { return myTriggerCamActive; }

		const bool IsCameraActive(size_t aIndex);

		void DoCameraShake(Volt::Entity aCameraEntity, const CameraShakeSettings& aShakeSetting);

		//Camera Settings
		void SetCameraLookAt(Volt::Entity aCamera, Volt::Entity aLookAtEnt);
		void SetCameraFollow(Volt::Entity aCamera, Volt::Entity aFollowEntity);
		void SetCameraFocusPoint(Volt::Entity aCamera, Volt::Entity aFocusEntity);
		void SetCameraDampAmount(Volt::Entity aCamera, float aDampAmount);
		void SetCameraFieldOfView(Volt::Entity aCamera, float aFoV);
		void SetCameraLocked(Volt::Entity aCamera, bool locked);
		//Camera Settings


	private:
		Scene* myScene = nullptr;

		void InitTransitionCam();
		void InitTransitionCam(const float& aBlendTime, const Volt::eBlendType blendType);

		Volt::Entity CreateTransitionCamera();

		void ShakeCamera(const float aDeltaTime);
		void BlendCameras(float aDeltaTime);

		std::vector<Volt::Entity> myVTCams;

		Volt::Entity myActiveCamera = Volt::Entity{ 0,nullptr };
		Volt::Entity myLastActiveCamera = Volt::Entity{ 0,nullptr };

		Volt::Entity myCurrentShakeCamera = Volt::Entity{ 0,nullptr };

		Volt::Entity myTransitionCamera = Volt::Entity{ 0,nullptr };
		Volt::Entity myActiveTiggerCamera = Volt::Entity{ 0,nullptr };

		gem::vec3 myNoiseSeeds = { 0 };

		gem::vec3 myTransitionCamStartPos = { 0 };
		gem::quat myTransitionCamStartRot = gem::quat(1, 0, 0, 0);
		float myTransitionCameraStartFoV = 0.f;

		CameraShakeSettings myCurrentShakeSetting;

		bool myIsShaking = false;
		bool myIsBlending = false;
		bool myTriggerCamActive = false;

		float myNoiseStep = 0.f;
		float myShakeTime = 0.f;
		float myMaxShakeMag = 0.f;
		float myCurrentShakeMag = 0.f;

		float myLerpTime = 0.f;
		float myCurrentLerpTime = 0.f;
	};
}