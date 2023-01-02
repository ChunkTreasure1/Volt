#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Script.h"

class Director : public Volt::Script
{
public:
	Director(const Volt::Entity& aEntity);
	~Director() override { myInstance = nullptr; };

	inline static Director& Get() { return *myInstance; }

	void SetActiveCamera(std::string aCamName);
	void SetActiveCamera(size_t aIndex);

	const bool IsCameraActive(size_t aIndex);

	const std::vector<Volt::Entity> GetAllCamerasInScene();

	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<Director>(aEntity); }
	static WireGUID GetStaticGUID() { return "{E858429A-6F96-450A-84FC-9803C2773C09}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	inline static Director* myInstance;

	void OnAwake() override;
	void OnUpdate(float aDeltaTime) override;

	void InitTransitionCam();
	Volt::Entity CreateTransitionCamera();

	void BlendCameras(float aDeltaTime);

	std::vector<Volt::Entity> myVTCams;

	Volt::Entity myActiveCamera = Volt::Entity{ 0,nullptr };
	Volt::Entity myTransitionCamera = Volt::Entity{ 0,nullptr };

	gem::vec3 myTransitionCamStartPos = { 0 };
	gem::vec3 myTransitionCamStartRot = { 0 };
	float myTransitionCameraStartFoV = 0.f;

	bool myIsBlending = false;

	float myLerpTime = 0.f;
	float myCurrentLerpTime = 0.f;
};