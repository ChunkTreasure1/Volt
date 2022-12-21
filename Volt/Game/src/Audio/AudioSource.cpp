#include "gepch.h"
#include "AudioSource.h"
#include <Volt/Core/Application.h>

#include <Volt/Input/KeyCodes.h>
#include <Volt/Scripting/ScriptRegistry.h>

VT_REGISTER_SCRIPT(AudioSourceScript);
AudioSourceScript::AudioSourceScript(Volt::Entity entity) : Volt::ScriptBase(entity){}

void AudioSourceScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(AudioSourceScript::OnKeyEvent));
	dispatcher.Dispatch<Volt::OnSceneStopEvent>(VT_BIND_EVENT_FN(AudioSourceScript::OnStopSceneEvent));
}

bool AudioSourceScript::OnKeyEvent(Volt::KeyPressedEvent& e)
{
	if (e.GetKeyCode() == VT_KEY_1)
	{
		Play("event:/TestDoot2D");
	}
	if (e.GetKeyCode() == VT_KEY_2)
	{
		Play("event:/TestDoot3D");
	}
	if (e.GetKeyCode() == VT_KEY_3)
	{
		Play("event:/TestTone2DLoop");
	}
	if (e.GetKeyCode() == VT_KEY_4)
	{
		Play("event:/TestTone3DLoop");
	}
	return false;
}

bool AudioSourceScript::OnStopSceneEvent(Volt::OnSceneStopEvent& e)
{
	StopAll();
	return false;
}


void AudioSourceScript::OnUpdate(float aDeltaTime)
{
	for (auto inst : instances3DMap)
	{
		inst.second.position = { myEntity.GetPosition().x, myEntity.GetPosition().y, myEntity.GetPosition().z };
		inst.second.forward = { myEntity.GetForward().x, myEntity.GetForward().y, myEntity.GetForward().z };
		inst.second.up = { myEntity.GetUp().x, myEntity.GetUp().y, myEntity.GetUp().z };
	}
}

bool AudioSourceScript::Play(const std::string& aPath)
{
	FMOD::Studio::EventDescription* fmodEventDesc = Amp::AudioManager::FindEvent(aPath);
	if(fmodEventDesc == nullptr)
	{
		return false;
	}

	bool is3D = false;
	fmodEventDesc->is3D(&is3D);
	Amp::EventData eventData;
	if (is3D)
	{
		eventData.is3D = true;
		eventData.position = { myEntity.GetPosition().x, myEntity.GetPosition().y, myEntity.GetPosition().z };
		eventData.forward = { myEntity.GetForward().x, myEntity.GetForward().y, myEntity.GetForward().z };
		eventData.up = { myEntity.GetUp().x, myEntity.GetUp().y, myEntity.GetUp().z };
	}

	bool isOneShot = false;
	fmodEventDesc->isOneshot(&isOneShot);
	if (isOneShot && !is3D)
	{
		lastPlayedOneShot = true;
		return Amp::AudioManager::PlayOneShot(aPath);
	}
	else if (isOneShot && is3D)
	{
		lastPlayedOneShot = true;
		return Amp::AudioManager::PlayOneShot(aPath, eventData);
	}

	Amp::EventInstance& instance = Amp::AudioManager::CreateEventInstance(aPath);
	bool isOK = Amp::AudioManager::PlayEvent(instance);

	if (!isOK) { return false; }
	lastPlayedOneShot = false;
	if (is3D)
	{
		instanceIDpool++;
		instances3DMap.insert({instanceIDpool, instance });
	}
	else
	{
		instanceIDpool++;
		instances2DMap.insert({ instanceIDpool, instance });
	}

	return isOK;
}

bool AudioSourceScript::Play(const int aID)
{
	if (auto It = instances2DMap.find(aID); It != instances2DMap.end())
	{
		Amp::AudioManager::PlayEvent(It->second);
		return true;
	}

	if (auto It = instances3DMap.find(aID); It != instances3DMap.end())
	{
		Amp::AudioManager::PlayEvent(It->second);
		return true;
	}
	return false;
}

int AudioSourceScript::GetLatestID()
{
	return instanceIDpool;
}

bool AudioSourceScript::Stop(int aID)
{
	if (auto It = instances2DMap.find(aID); It != instances2DMap.end())
	{
		Amp::AudioManager::StopEvent(It->second, FMOD_STUDIO_STOP_IMMEDIATE);
	}

	if (auto It = instances3DMap.find(aID); It != instances3DMap.end())
	{
		Amp::AudioManager::StopEvent(It->second, FMOD_STUDIO_STOP_IMMEDIATE);
	}

	return false;
}

void AudioSourceScript::StopAll()
{
	for (auto inst : instances2DMap)
	{
		Amp::AudioManager::RemoveEvent(inst.second);
	}
	instances2DMap.clear();
	for (auto inst : instances3DMap)
	{
		Amp::AudioManager::RemoveEvent(inst.second);
	}
	instances3DMap.clear();
	instanceIDpool = -1;
}