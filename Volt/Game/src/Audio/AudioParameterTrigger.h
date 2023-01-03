#pragma once
#include <Volt/Scripting/Script.h>
#include <Audio/AudioEventEmitter.h>

class AudioParameterTriggerScript : public Volt::Script
{
public:
	AudioParameterTriggerScript(Volt::Entity entity);
	~AudioParameterTriggerScript() override = default;

	//void OnAwake() override;
	void OnStart() override;
	void OnDetach() override;
	//void OnStop() override;
	//void OnUpdate(float aDeltaTime) override;
	//void OnEvent(Volt::Event& e) override;
	//bool OnKeyEvent(Volt::KeyPressedEvent& e);

	void OnCollisionEnter(Volt::Entity) override;
	void OnCollisionExit(Volt::Entity) override;

	void OnTriggerEnter(Volt::Entity, bool) override;
	void OnTriggerExit(Volt::Entity, bool) override;

	void TriggerParameterChange();

	static WireGUID GetStaticGUID() { return "{3D71811B-7D2E-4CE4-A216-5C0E0D587C41}"_guid; }
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:

	Amp::TriggerType triggerType = Amp::TriggerType::NONE;
	std::string parameterPath = "";
	float parameterValue = 0.0f;
	bool isTriggerOnce = false;
	bool isTriggered = false;

	AudioEventEmitterScript* eventEmitter = nullptr;
};