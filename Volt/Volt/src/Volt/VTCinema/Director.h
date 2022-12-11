#pragma once
#include "Volt/Scene/Entity.h"
#include "Volt/Scripting/ScriptBase.h"

class Director : public Volt::ScriptBase
{
public:
	Director(const Volt::Entity& aEntity);

	const inline static Ref<Director> Get() { return myInstance; }

	void SetActiveCamera(std::string aCamName);
	void SetActiveCamera(size_t aIndex);

	static Ref<ScriptBase> Create(Volt::Entity aEntity) { return CreateRef<Director>(aEntity); }
	static WireGUID GetStaticGUID() { return "{E858429A-6F96-450A-84FC-9803C2773C09}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	inline static Ref<Director> myInstance;

	void OnAwake() override;
	void OnUpdate(float aDeltaTime) override;
	void InitTransitionCam();

	std::vector<Volt::Entity> myVTCams;

	Volt::Entity myActiveCamera = Volt::Entity{ 0,nullptr };

	bool myChangeCamera = false;
	bool myIsChangingCamera = false;

};