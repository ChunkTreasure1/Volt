#pragma once
#include <Amp/AudioEngine/AudioEngine.h>

namespace Amp
{
	class AudioManager 
	{
	public:
		//Engine Handling
		static void Init(InitInsturct aSetUpInstruction);
		static void Shutdown();

		//Listener Handling
		static bool InitListener(int ID);
		static void Update();
		static void UpdateListener(ListenerData aListenerData);

		//Event Handling
		static bool CreateEventInstance(std::string aPath, EventInstance& aEvent);
		static bool PlayEvent(EventInstance& aEvent);
		static bool RemoveEvent(EventInstance& aEvent);

		//Mixer Handling
		static bool SetMasterVolume(float aVolPerct);
		static bool SetMixerVolume(std::string aMixerPath, float aVolPerct);

	private:
		inline static AudioEngine myAudioEngine = AudioEngine();
	};
}