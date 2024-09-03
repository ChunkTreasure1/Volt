//#pragma once
//#include "Volt/Scene/Entity.h"
//#include "Volt/Scripting/Script.h"
//
//class VisionTrigger : public Volt::Script
//{
//public:
//	VisionTrigger(const Volt::Entity& aEntity);
//
//	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<VisionTrigger>(aEntity); }
//	static WireGUID GetStaticGUID() { return "{B558050D-3E87-4CC5-94A2-6EBC364833A2}"_guid; };
//	WireGUID GetGUID() override { return GetStaticGUID(); }
//
//private:
//	void OnAwake() override;
//	void OnUpdate(float aDeltaTime) override;
//	void OnTriggerEnter(Volt::Entity entity, bool isTrigger) override;
//	void OnTriggerExit(Volt::Entity entity, bool isTrigger) override;
//
//	Volt::Entity myTriggerCam = Volt::Entity{ 0,nullptr };
//};