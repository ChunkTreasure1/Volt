#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class CreateEntityNode : public Node
	{
	public:
		CreateEntityNode();

		inline const std::string GetName() override { return "Create Entity"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
		
	private:
		void CreateEntity();
	};

	class EntityNode : public Node
	{
	public:
		EntityNode();

		inline const std::string GetName() override { return "Entity"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
	};
}