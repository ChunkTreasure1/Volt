#pragma once

#include "GraphKey/Node.h"

#include <Volt/Scene/Entity.h>

#include <glm/glm.hpp>

namespace GraphKey
{
	class GetEntityTransformNode : public Node
	{
	public:
		GetEntityTransformNode();
		~GetEntityTransformNode() override = default;

		inline const std::string GetName() override { return "Get Entity Transform"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

		void Initialize() override;

	private:
		void GetPosition();
		void GetRotation();
		void GetScale();
	};

	class SetEntityTransformNode : public Node
	{
	public:
		SetEntityTransformNode();
		~SetEntityTransformNode() override = default;

		inline const std::string GetName() override { return "Set Entity Transform"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

		void Initialize() override;

	private:
		void SetTransform();
	};

	class SetEntityPositionNode : public Node
	{
	public:
		SetEntityPositionNode();

		~SetEntityPositionNode() override = default;

		inline const std::string GetName() override { return "Set Entity Position"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

		void Initialize() override;

	private:
		void SetPosition();
	};

	class SetEntityRotationNode : public Node
	{
	public:
		SetEntityRotationNode();
		~SetEntityRotationNode() override = default;

		inline const std::string GetName() override { return "Set Entity Rotation"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

		void Initialize() override;

	private:
		void SetRotation();
	};

	class AddEntityRotationNode : public Node
	{
	public:
		AddEntityRotationNode();
		~AddEntityRotationNode() override = default;

		inline const std::string GetName() override { return "Add Entity Rotation"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

		void Initialize() override;

	private:
		void AddRotation();
	};
}
