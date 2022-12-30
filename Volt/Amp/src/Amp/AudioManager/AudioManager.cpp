#include "amppch.h"
#include "AudioManager.h"

namespace Amp
{
	void AudioManager::Init(InitInsturct aSetUpInstruction)
	{
		myAudioEngine.Init(aSetUpInstruction.aFileDirectory);
		myAudioEngine.LoadMasterBank(aSetUpInstruction.aMasterbank, aSetUpInstruction.aMasterStringsBank, aSetUpInstruction.aLoadBankFlags);
	}
	void AudioManager::Shutdown()
	{
		myAudioEngine.Release();
	}

	void AudioManager::ReleaseAll() 
	{
		myAudioEngine.ReleaseAll();
	}

	bool AudioManager::InitListener(int ID = 0)
	{
		//TODO: Add check if listeners exists

		ListenerData aNewListener;
		aNewListener.ID = ID;
		
		return myAudioEngine.InitListener(aNewListener);
	}

	void AudioManager::Update(float aDeltaTime)
	{
		myAudioEngine.Update(aDeltaTime);
	}

	void AudioManager::UpdateListener(ListenerData aListenerData)
	{

		myAudioEngine.UpdateListener(aListenerData);
	}

	FMOD::Studio::EventDescription* AudioManager::FindEvent(const std::string& aPath)
	{
		return myAudioEngine.FindEvent(aPath);
	}

	EventInstance& AudioManager::CreateEventInstance(std::string aPath)
	{
		return myAudioEngine.CreateEventInstance(aPath);
	}

	bool AudioManager::PlayEvent(EventInstance& aEvent)
	{
		return myAudioEngine.PlayEvent(aEvent.instance);
	}

	bool AudioManager::RemoveEvent(EventInstance& aEvent)
	{
		return myAudioEngine.RemoveEvent(aEvent);
	}

	bool AudioManager::StopEvent(EventInstance& aEvent, FMOD_STUDIO_STOP_MODE aMode)
	{
		return myAudioEngine.StopEvent(aEvent.instance, aMode);
	}

	bool AudioManager::PauseEvent(EventInstance& aEvent)
	{
		return myAudioEngine.PauseEvent(aEvent.instance);
	}

	bool AudioManager::UnpauseEvent(EventInstance& aEvent)
	{
		return myAudioEngine.UnpauseEvent(aEvent.instance);
	}

	bool AudioManager::PlayOneShot(const std::string& aPath)
	{
		EventData aEmptyData;
		aEmptyData.position = { 0,0,0 };
		aEmptyData.forward = { 0,0,0 };
		aEmptyData.up = { 0,0,0 };

		return myAudioEngine.PlayOneShot(aPath, aEmptyData);
	}

	bool AudioManager::PlayOneShot(const std::string& aPath, EventData& aEventData)
	{
		return myAudioEngine.PlayOneShot(aPath, aEventData);
	}

	bool AudioManager::SetEventParameter(EventInstance& aInstance, EventData& aEventData)
	{
		return myAudioEngine.SetEventParameter(aInstance.instance, aEventData.aParameterPath, aEventData.aParameterValue);
	}

	bool AudioManager::SetEventParameter(EventInstance& aInstance, const std::string& aParameterPath, const float aValue)
	{
		return myAudioEngine.SetEventParameter(aInstance.instance, aParameterPath, aValue);
	}
}

