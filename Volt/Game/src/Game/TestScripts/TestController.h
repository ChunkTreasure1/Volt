#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/Script.h"
#include "Volt/Log/Log.h"

class TestController : public Volt::Script
{
public:
	TestController(const Volt::Entity& aEntity);

	void DoCameraSequence();
		
	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<TestController>(aEntity); }
	static WireGUID GetStaticGUID() { return "{2EF02AA8-E13C-4C5D-AB5E-78DCD24C0F13}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:

	Volt::Entity myLookCamera = Volt::Entity{ 0,nullptr };
	Volt::Entity myAimCamera = Volt::Entity{ 0,nullptr };

	bool myShouldAim = false;
	bool myIsAiming = false;

	bool myCameraSequenceOn = false;

	size_t myCamIndex = 0;

	float myMoveSpeed = 0.f;

	void OnAwake() override;
	void OnUpdate(float aDeltaTime) override;
};