#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Script.h"

class VTCameraController : public Volt::Script
{
public:
	VTCameraController(const Volt::Entity& aEntity);

	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<VTCameraController>(aEntity); }
	static WireGUID GetStaticGUID() { return "{B55A143E-CE4B-47C3-A44E-1F3F8A111B10}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	void OnAwake() override;
	void OnUpdate(float aDeltaTime) override;

	void FreeController(float aDeltaTime);
	void TPSController(float aDeltaTime);

	float myYawDelta = { 0 };
	float myPitchDelta = { 0 };
	gem::vec2 myLastMousePos = { 0 };
};