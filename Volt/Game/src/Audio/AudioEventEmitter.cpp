#include "gepch.h"
#include "AudioEventEmitter.h"
#include <Volt/Scripting/ScriptRegistry.h>
#include <Volt/Components/Components.h>

VT_REGISTER_SCRIPT(AudioEventEmitterScript);
AudioEventEmitterScript::AudioEventEmitterScript(Volt::Entity entity) : Volt::ScriptBase(entity) {}

void AudioEventEmitterScript::OnStart()
{
	audioSource = myEntity.GetScript<AudioSourceScript>();
	if (audioSource == nullptr)
	{
		myEntity.AddScript("AudioSourceScript");
		audioSource = myEntity.GetScript<AudioSourceScript>();
	}

	//TODO:: Check if it has Rigidbody
	//TODO:: Check if it has CollisionComponent

	//TODO:: Check if it has component
	auto emitterComponent = myEntity.GetComponent<Volt::AudioEventEmitterComponent>();
	playTrigger = static_cast<Amp::TriggerType>(emitterComponent.playTriggerCondition);
	stopTrigger = static_cast<Amp::TriggerType>(emitterComponent.stopTriggerCondition);
	isTriggerOnce = emitterComponent.isTriggerOnce;
	eventPath = emitterComponent.eventPath;

	if (playTrigger == Amp::TriggerType::START) 
	{
		PlayEvent();
	}
}

void AudioEventEmitterScript::OnDetach()
{
	if (playTrigger == Amp::TriggerType::DESTROY)
	{
		PlayEvent();
	}
}

void AudioEventEmitterScript::OnCollisionEnter(Volt::Entity)
{
	if (playTrigger == Amp::TriggerType::COLLISIONENTER)
	{
		PlayEvent();
	}
	else if(stopTrigger == Amp::TriggerType::COLLISIONENTER)
	{
		StopEvent();
	}
}

void AudioEventEmitterScript::OnCollisionExit(Volt::Entity)
{
	if (playTrigger == Amp::TriggerType::COLLISIONEXIT)
	{
		PlayEvent();
	}
	else if (stopTrigger == Amp::TriggerType::COLLISIONEXIT)
	{
		StopEvent();
	}
}

void AudioEventEmitterScript::OnTriggerEnter(Volt::Entity, bool)
{
	if (playTrigger == Amp::TriggerType::TRIGGERENTER)
	{
		PlayEvent();
	}
	else if (stopTrigger == Amp::TriggerType::TRIGGERENTER)
	{
		StopEvent();
	}
}

void AudioEventEmitterScript::OnTriggerExit(Volt::Entity, bool)
{
	if (playTrigger == Amp::TriggerType::TRIGGEREXIT)
	{
		PlayEvent();
	}
	else if (stopTrigger == Amp::TriggerType::TRIGGEREXIT)
	{
		StopEvent();
	}
}

void AudioEventEmitterScript::PlayEvent()
{
	if (isPlayTriggered) { return; }
	if (playingEventID >= 0 && currentStatus == Amp::EmitterStatus::STOPPED)
	{
		audioSource->Play(playingEventID);
		currentStatus = Amp::EmitterStatus::PLAYING;
	}
	else 
	{
		audioSource->Play(eventPath);
		currentStatus = Amp::EmitterStatus::PLAYING;
		playingEventID = audioSource->GetLatestID();
		if (isTriggerOnce) { isPlayTriggered = true; }
	}
}

void AudioEventEmitterScript::StopEvent()
{
	if (isStopTriggered) { return; }
	if(playingEventID >= 0 && currentStatus == Amp::EmitterStatus::PLAYING)
	{
		currentStatus = Amp::EmitterStatus::STOPPED;
		audioSource->Stop(playingEventID);
		if (isTriggerOnce) { isStopTriggered = true; }
	}
}


