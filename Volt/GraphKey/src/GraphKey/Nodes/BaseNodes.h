#pragma once

#include "GraphKey/Node.h"

#include <Volt/Events/ApplicationEvent.h>

namespace GraphKey
{
	class StartNode : public Node
	{
	public:
		StartNode();
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "Start"; }

		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};

	class UpdateNode : public Node
	{
	public:
		UpdateNode();
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "Update"; }

		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};
}