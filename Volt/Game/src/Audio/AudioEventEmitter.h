#pragma once
#include <Volt/Scripting/Script.h>
#include <Audio/AudioSource.h>

namespace Amp
{
	enum class EmitterStatus 
	{
		WAITING,
		PLAYING,
		STOPPED
	};
}

class AudioEventEmitterScript : public Volt::Script
{
public:
	AudioEventEmitterScript(Volt::Entity entity);
	~AudioEventEmitterScript() override = default;

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

	void PlayEvent();
	void StopEvent();

	Amp::EventInstance& GetEventInstance();

	static WireGUID GetStaticGUID() { return "{2CE06BC9-2BFE-46AD-84CF-B272458735C7}"_guid; }
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:

	Amp::TriggerType playTrigger = Amp::TriggerType::NONE;
	Amp::TriggerType stopTrigger = Amp::TriggerType::NONE;

	std::string eventPath = "";

	bool isTriggerOnce = false;
	bool isPlayTriggered = false;
	bool isStopTriggered = false;

	AudioSourceScript* audioSource = nullptr;

	int playingEventID = -1;
	Amp::EmitterStatus currentStatus = Amp::EmitterStatus::WAITING;

};