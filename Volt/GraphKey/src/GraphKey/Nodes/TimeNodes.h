#pragma once

#include "GraphKey/Node.h"
#include "GraphKey/TimerManager.h"

namespace GraphKey
{
	class StartTimerNode : public Node
	{
	public:
		inline StartTimerNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(StartTimerNode::StartTimer)),
				AttributeConfig<float>("Time", AttributeDirection::Input)
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output),
				AttributeConfig("Timer Done", AttributeDirection::Output),
				AttributeConfig<Volt::UUID>("ID", AttributeDirection::Output, true)
			};
		}

		inline const std::string GetName() override { return "Start Timer"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void StartTimer()
		{
			const auto id = TimerManager::AddTimer(GetInput<float>(1), [&]()
			{
				ActivateOutput(1);
			});

			SetOutputData(2, id);
			ActivateOutput(0);
		}
	};

	class StopTimerNode : public Node
	{
	public:
		inline StopTimerNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(StopTimerNode::StopTimer)),
				AttributeConfig<Volt::UUID>("ID", AttributeDirection::Input, true)
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output)
			};
		}

		inline const std::string GetName() override { return "Stop Timer"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }

	private:
		inline void StopTimer()
		{
			const auto id = GetInput<Volt::UUID>(1);

			TimerManager::StopTimer(id);
			ActivateOutput(0);
		}
	};
}