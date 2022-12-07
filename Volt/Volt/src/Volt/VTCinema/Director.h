#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/ScriptBase.h"

class Director : public Volt::ScriptBase
{
public:
	Director(const Volt::Entity& aEntity);

	static Ref<ScriptBase> Create(Volt::Entity aEntity) { return CreateRef<Director>(aEntity); }
	static WireGUID GetStaticGUID() { return "{E858429A-6F96-450A-84FC-9803C2773C09}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	
	void OnAwake() override;
	void OnUpdate(float aDeltaTime) override;
	
};