#pragma once

#include "Event.h"

#include "Volt/Asset/Asset.h"

#include <sstream>

#include <filesystem>

namespace Volt
{
	class OnRespawnEvent : public Event
	{
	public:
		OnRespawnEvent() = default;

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnRespawn" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnRespawn);
		EVENT_CLASS_CATEGORY(EventCategoryGame);
	};

	class OnGameStateChangedEvent : public Event
	{
	public:

		enum State
		{
			PLAY,
			PAUSE
		};

		OnGameStateChangedEvent(const State aState) : m_State(aState) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnGameStateChanged" << std::endl;
			return ss.str();
		}

		inline const State GetState() { return m_State; }

		EVENT_CLASS_TYPE(OnGameStateChanged);
		EVENT_CLASS_CATEGORY(EventCategoryGame);

	private:
		State m_State;
	};

	class OnPlayGameEvent : public Event
	{
	public:
		OnPlayGameEvent(Volt::AssetHandle aLevelHandle)
			: myLevelHandle(aLevelHandle)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnPlayGame" << std::endl;
			return ss.str();
		}

		inline Volt::AssetHandle GetHandle() const { return myLevelHandle; }

		EVENT_CLASS_TYPE(OnPlayGame);
		EVENT_CLASS_CATEGORY(EventCategoryGame);

	private:
		Volt::AssetHandle myLevelHandle;

	};

}