#pragma once

#include "GraphKey/Node.h"

#include <Volt/Components/LightComponents.h>

namespace GraphKey
{
	class LightNode : public Node
	{
	public:
		inline LightNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(LightNode::SetProperties)),
				AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Input),
				AttributeConfigDefault<float>("Intensity", AttributeDirection::Input, 1.f),
				AttributeConfigDefault<float>("Radius", AttributeDirection::Input, 100.f),
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output),
			};
		}

		~LightNode() override = default;

		void Initialize() override;

		inline const std::string GetName() override { return "Light"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:

		void SetProperties();
	};

	class ChangeLightColorNode : public Node
	{
	public:
		inline ChangeLightColorNode()
		{
			inputs =
			{
				AttributeConfig("", AttributeDirection::Input, GK_BIND_FUNCTION(ChangeLightColorNode::SetProperties)),
				AttributeConfig<Volt::Entity>("Entity", AttributeDirection::Input),
				AttributeConfigDefault("Color", AttributeDirection::Input, glm::vec3{ 1.f }),
				AttributeConfigDefault("Time", AttributeDirection::Input, 1.f)
			};

			outputs =
			{
				AttributeConfig("", AttributeDirection::Output),
			};
		}

		~ChangeLightColorNode() override = default;

		void Initialize() override;

		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "Change Light Color"; }
		inline const glm::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }

	private:
		bool myHasStartedLerp = false;
		float myLerpTime = 0.f;
		float myTotalLerpTime = 0.f;

		glm::vec3 myTargetColor{};
		glm::vec3 myStartColor{};

		Volt::Entity myTargetEntity{};

		void SetProperties();
	};
}
