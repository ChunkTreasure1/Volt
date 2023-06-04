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
		static void UpdateListener(ListenerData& aListenerData);

		//Event Handling
		static EventInstance& CreateEventInstance(std::string aPath);
		static bool PlayEvent(EventInstance& aEvent);
		static bool RemoveEvent(EventInstance& aEvent);
		static bool StopEvent(EventInstance& aEvent, FMOD_STUDIO_STOP_MODE aMode);
		static void StopAllEvents(const int aStopMode);
		static bool PauseEvent(EventInstance& aEvent);
		static bool UnpauseEvent(EventInstance& aEvent);

		static FMOD::Studio::EventDescription* FindEvent(const std::string&);
		static std::vector<std::string> GetAllEventNames();

		static bool PlayOneShot(const std::string& aPath);
		static bool PlayOneShot(const std::string& aPath, EventData& aEventData);

		//Parameter Handling
		static bool SetEventParameter(EventInstance& aInstance, EventData& aEventData);
		static bool SetEventParameter(EventInstance& aInstance, const std::string& aParameterPath, const float aValue);

		//Mixer Handling
		static bool SetMasterVolume(float aVolPerct);
		static bool SetMixerVolume(std::string aMixerPath, float aVolPerct);

	private:
		static bool EventNameSorter(std::string a, std::string b) { return a < b; }

	private:
		inline static AudioEngine myAudioEngine = AudioEngine();
	};
}
