#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	struct CustomEventNode : public Node
	{
		~CustomEventNode() override = default;
		Volt::UUID eventId;
	};

	struct CallCustomEventNode : public CustomEventNode
	{
		CallCustomEventNode();

		inline ~CallCustomEventNode() override = default;

		const std::string GetName() override;

		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void CallEvent();
	};

	struct RecieveCustomEventNode : public CustomEventNode
	{
		RecieveCustomEventNode();

		void OnCopy() override;
		~RecieveCustomEventNode() override;

		inline const std::string GetName() override { return "On " + myGraph->GetEventNameFromId(eventId); }
		inline const glm::vec4 GetColor() override { return { 1.f }; }
	};
}
