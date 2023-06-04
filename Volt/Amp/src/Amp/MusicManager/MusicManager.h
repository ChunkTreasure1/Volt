#pragma once
#include <Amp/AudioManager/AudioManager.h>

namespace Amp
{
	class MusicManager
	{
	public:
		enum class CurrentState
		{
			WAITING,
			PLAYING,
			STOPPED,
			PAUSED,
			FINISHED,
			REMOVED
		};

		static bool CreateMusicInstance(std::string aPath);
		static bool Play();
		static bool Stop();
		static bool Pause();
		static bool Unpause();
		static bool RemoveEvent();

		static bool SetEventParameter(const std::string& aParameterPath, const float aValue);

		static CurrentState GetCurrentState();

	private:
		inline static std::string currentAudio;
		inline static EventInstance loadedAudio;
		inline static CurrentState currentState = CurrentState::WAITING;

	};
}
