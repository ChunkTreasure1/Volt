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

	bool AudioManager::InitListener(int ID = 0)
	{
		//TODO: Add check if listeners exists

		ListenerData aNewListener;
		aNewListener.ID = ID;
		
		return myAudioEngine.InitListener(aNewListener);
	}

	void AudioManager::Update()
	{
		myAudioEngine.Update();
	}

	void AudioManager::UpdateListener(ListenerData aListenerData)
	{
		myAudioEngine.UpdateListener(aListenerData);
	}

	bool AudioManager::CreateEventInstance(std::string aPath, EventInstance& aEvent)
	{
		aEvent = myAudioEngine.CreateEventInstance(aPath);

		return false;
	}
	bool AudioManager::PlayEvent(EventInstance& aEvent)
	{
		myAudioEngine.PlayEvent(aEvent.instance);
		return false;
	}
	bool AudioManager::RemoveEvent(EventInstance& aEvent)
	{
		return false;
	}
}

