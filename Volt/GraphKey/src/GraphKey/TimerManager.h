#pragma once

#include <Volt/Core/UUID.h>

#include <unordered_map>
#include <functional>

namespace GraphKey
{
	class TimerManager
	{
	public:
		inline static void Update(float deltaTime)
		{
			std::vector<Volt::UUID> timersToRemove{};

			for (auto& [id, timer] : myTimers)
			{
				timer.currentTime -= deltaTime;
				if (timer.currentTime <= 0.f)
				{
					timer.endFunction();
					timersToRemove.emplace_back(id);
				}
			}

			for (const auto& id : timersToRemove)
			{
				myTimers.erase(id);
			}
		}

		inline static void Clear()
		{
			myTimers.clear();
		}

		inline static const Volt::UUID AddTimer(const float time, std::function<void()>&& endFunction)
		{
			Volt::UUID id{};
			myTimers.emplace(id, Timer{ time, std::move(endFunction) });

			return id;
		}

		inline static void StopTimer(const Volt::UUID id)
		{
			if (!myTimers.contains(id))
			{
				return;
			}

			myTimers.erase(id);
		}

	private:
		TimerManager() = delete;

		struct Timer
		{
			float currentTime = 0.f;
			std::function<void()> endFunction;
		};

		inline static std::unordered_map<Volt::UUID, Timer> myTimers;
	};
}