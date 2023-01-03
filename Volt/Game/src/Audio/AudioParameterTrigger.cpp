#include "gepch.h"
#include "AudioParameterTrigger.h"
#include <Volt/Scripting/ScriptRegistry.h>
#include <Volt/Components/Components.h>

VT_REGISTER_SCRIPT(AudioParameterTriggerScript);
AudioParameterTriggerScript::AudioParameterTriggerScript(Volt::Entity entity) : Volt::ScriptBase(entity) {}

void AudioParameterTriggerScript::OnStart()
{
	Volt::Entity tempEnt;
	myEntity.GetScene()->GetRegistry().ForEach<Volt::AudioParameterTriggerComponent>([&](Wire::EntityId id, const Volt::AudioParameterTriggerComponent& ParameterTrigger)
		{
			tempEnt = Volt::Entity(id, myEntity.GetScene());
			return;
		});

	eventEmitter = tempEnt.GetScript<AudioEventEmitterScript>();

	//TODO:: Check if it has Rigidbody
	//TODO:: Check if it has CollisionComponent

	//TODO:: Check if it has component
	auto parameterComponent = myEntity.GetComponent<Volt::AudioParameterTriggerComponent>();
	triggerType = static_cast<Amp::TriggerType>(parameterComponent.triggerCondition);
	isTriggerOnce = parameterComponent.isTriggerOnce;
	parameterPath = parameterComponent.eventPath;

	if (triggerType == Amp::TriggerType::START)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::OnDetach()
{
	if (triggerType == Amp::TriggerType::DESTROY)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::OnCollisionEnter(Volt::Entity)
{
	if (triggerType == Amp::TriggerType::COLLISIONENTER)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::OnCollisionExit(Volt::Entity)
{
	if (triggerType == Amp::TriggerType::COLLISIONEXIT)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::OnTriggerEnter(Volt::Entity, bool)
{
	if (triggerType == Amp::TriggerType::TRIGGERENTER)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::OnTriggerExit(Volt::Entity, bool)
{
	if (triggerType == Amp::TriggerType::TRIGGEREXIT)
	{
		TriggerParameterChange();
	}
}

void AudioParameterTriggerScript::TriggerParameterChange()
{
	if (isTriggered) { return; }
	Amp::EventInstance& instance = eventEmitter->GetEventInstance();
	Amp::AudioManager::SetEventParameter(instance, parameterPath, parameterValue);
	if (isTriggerOnce) { isTriggered = true; }
}

