#include "amppch.h"
#include "MusicManager.h"

bool Amp::MusicManager::CreateMusicInstance(std::string aPath)
{
	loadedAudio.instance->release();
	loadedAudio = AudioManager::CreateEventInstance(aPath);
	currentAudio = aPath;
    return true;
}

bool Amp::MusicManager::Play()
{
	bool success = AudioManager::PlayEvent(loadedAudio);
	if (success) { currentState = CurrentState::PLAYING; }
	return success;
}

bool Amp::MusicManager::Stop()
{
	bool success = AudioManager::StopEvent(loadedAudio, FMOD_STUDIO_STOP_ALLOWFADEOUT);
	if (success) { currentState = CurrentState::STOPPED; }
	return success;
}

bool Amp::MusicManager::Pause()
{
	bool success = AudioManager::PauseEvent(loadedAudio);
	if (success) { currentState = CurrentState::PAUSED; }
	return success;
}

bool Amp::MusicManager::Unpause()
{
	bool success = AudioManager::UnpauseEvent(loadedAudio);
	if (success) { currentState = CurrentState::PLAYING; }
	return success;
}

bool Amp::MusicManager::RemoveEvent()
{
	MusicManager::Stop();

	bool success = AudioManager::RemoveEvent(loadedAudio);
	if (success) 
	{
		currentState = CurrentState::REMOVED;
		currentAudio = "";
		loadedAudio = EventInstance();
	}
	return success;
}

bool Amp::MusicManager::SetEventParameter(const std::string& aParameterPath, const float aValue)
{
	bool success = AudioManager::SetEventParameter(loadedAudio, aParameterPath, aValue);
	return success;
}

Amp::MusicManager::CurrentState Amp::MusicManager::GetCurrentState()
{
	return currentState;
}
