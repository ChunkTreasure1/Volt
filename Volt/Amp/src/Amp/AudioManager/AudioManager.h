#pragma once
#include <Amp/AudioEngine/AudioEngine.h>

namespace Amp
{
	enum class TriggerType
	{
		NONE = 0,
		START,
		DESTROY,
		TRIGGERENTER,
		TRIGGEREXIT,
		COLLISIONENTER,
		COLLISIONEXIT
	};

	class AudioManager 
	{
	public:
		//Engine Handling
		static void Init(InitInsturct aSetUpInstruction);
		static void Shutdown();
		static void ReleaseAll();

		//Listener Handling
		static bool InitListener(int ID);
		static void Update(float aDeltaTime);
		static void UpdateListener(ListenerData aListenerData);

		//Event Handling
		static EventInstance& CreateEventInstance(std::string aPath);
		static bool PlayEvent(EventInstance& aEvent);
		static bool RemoveEvent(EventInstance& aEvent);
		static bool StopEvent(EventInstance& aEvent, FMOD_STUDIO_STOP_MODE aMode);
		static void StopAll(const int aStopMode);

		static FMOD::Studio::EventDescription* FindEvent(const std::string&);

		static bool PlayOneShot(const std::string& aPath);
		static bool PlayOneShot(const std::string& aPath, EventData& aEventData);

		//Mixer Handling
		static bool SetMasterVolume(float aVolPerct);
		static bool SetMixerVolume(std::string aMixerPath, float aVolPerct);

	private:
		inline static AudioEngine myAudioEngine = AudioEngine();
	};
}