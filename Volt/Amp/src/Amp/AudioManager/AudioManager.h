#pragma once
#include <Amp/AudioEngine/AudioEngine.h>

namespace Amp
{
	struct InitInsturct 
	{
		std::string aFileDirectory;
		std::string aMasterbank;
		std::string aMasterStringsBank;
		FMOD_STUDIO_LOAD_BANK_FLAGS aLoadBankFlags;
	};

	struct ListenerData 
	{

	};

	struct EventData
	{
		int myId;
		std::string myName;
		FMOD::Studio::EventInstance* myEventInstance;
	};

	class AudioManager 
	{
	public:
		//Engine Handling
		static void Init(InitInsturct aSetUpInstruction);
		static void Shutdown();

		static void InitNewListener();

		static void Update();
		static void UpdateListener(ListenerData aListenerData);

		//Event Handling

		//Mixer Handling
		static bool SetMasterVolume(float aVolPerct);
		static bool SetMixerVolume(std::string aMixerPath, float aVolPerct);

	private:
		inline static std::vector<Listener> activeListeners;

		inline static AudioEngine myAudioEngine = AudioEngine();

	};
}