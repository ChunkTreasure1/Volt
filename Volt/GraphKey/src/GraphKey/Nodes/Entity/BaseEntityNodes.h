#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	class CreateEntityNode : public Node
	{
	public:
		CreateEntityNode();
		~CreateEntityNode() override = default;

		inline const std::string GetName() override { return "Create Entity"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		void CreateEntity();
	};

	class DestroyEntityNode : public Node
	{
	public:
		DestroyEntityNode();
		~DestroyEntityNode() override = default;

		inline const std::string GetName() override { return "Destroy Entity"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		void DestroyEntity();
	};

	class EntityNode : public Node
	{
	public:
		EntityNode();
		~EntityNode() override = default;

		void Initialize() override;

		inline const std::string GetName() override { return "Entity"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		void GetEntity();
	};

	class SelfNode : public Node
	{
	public:
		SelfNode();
		~SelfNode() override = default;

		void Initialize() override;

		inline const std::string GetName() override { return "Self"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		void GetEntity();
	};

	class GetChildCountNode : public Node
	{
	public:
		GetChildCountNode();
		~GetChildCountNode() override = default;

		void Initialize() override;

		inline const std::string GetName() override { return "Get Child Count"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		void GetChildCount();
	};
}
