#include "gepch.h"
#include <Volt/Scripting/ScriptRegistry.h>
#include "AudioListener.h"
#include <Amp/AudioManager/AudioManager.h>

VT_REGISTER_SCRIPT(AudioListenerScript);
AudioListenerScript::AudioListenerScript(Volt::Entity entity) : Volt::ScriptBase(entity) {}

void AudioListenerScript::OnUpdate(float aDeltaTime)
{
	Amp::ListenerData listenerData;
	listenerData.aDeltaTime = aDeltaTime;
	listenerData.ID = 0;
	listenerData.position = myEntity.GetPosition();
	listenerData.forward = myEntity.GetForward();
	listenerData.up = myEntity.GetUp();

	Amp::AudioManager::UpdateListener(listenerData);

}
