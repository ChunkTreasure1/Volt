#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class PrintNode : public Node
	{
	public:
		PrintNode();

		inline const std::string GetName() override { return "Print"; }
		inline const glm::vec4 GetColor() override { return { 1.f, 0.f, 0.f, 1.f }; }

	private:
		void Print();
	};
}
