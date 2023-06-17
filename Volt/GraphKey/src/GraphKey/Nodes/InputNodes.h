#pragma once

#include "GraphKey/Node.h"

#include <GEM/gem.h>

namespace GraphKey
{
	class KeyPressedNode : public Node
	{
	public:
		KeyPressedNode();
		~KeyPressedNode() override = default;

		const std::string GetName() override { return "Key Pressed"; }
		const gem::vec4 GetColor() override { return { 1.f, 0.37f, 0.53f, 1.f }; }

		void OnEvent(Volt::Event& e) override;
	};

	class KeyReleasedNode : public Node
	{
	public:
		KeyReleasedNode();
		~KeyReleasedNode() override = default;

		const std::string GetName() override { return "Key Released"; }
		const gem::vec4 GetColor() override { return { 1.f, 0.37f, 0.53f, 1.f }; }

		void OnEvent(Volt::Event& e) override;
	};
}
