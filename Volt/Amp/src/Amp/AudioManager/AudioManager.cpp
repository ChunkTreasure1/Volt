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
}

