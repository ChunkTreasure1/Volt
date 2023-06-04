#pragma once

#include <Volt/Events/Event.h>
#include <Volt/Scene/Entity.h>

#include "GraphKey/Node.h"

#include <sstream>

namespace GraphKey
{
	class OnCollisionEnterEvent : public Volt::Event
	{
	public:
		inline OnCollisionEnterEvent(Volt::Entity otherEntity)
			: myOtherEntity(otherEntity)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnCollisionEnter" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnCollisionEnter);
		EVENT_CLASS_CATEGORY(Volt::EventCategoryGraphKey);

		inline const Volt::Entity GetOther() const { return myOtherEntity; }

	private:
		Volt::Entity myOtherEntity;
	};

	class OnCollisionExitEvent : public Volt::Event
	{
	public:
		inline OnCollisionExitEvent(Volt::Entity otherEntity)
			: myOtherEntity(otherEntity)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnCollisionExit" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnCollisionExit);
		EVENT_CLASS_CATEGORY(Volt::EventCategoryGraphKey);

		inline const Volt::Entity GetOther() const { return myOtherEntity; }

	private:
		Volt::Entity myOtherEntity;
	};

	class OnTriggerExitEvent : public Volt::Event
	{
	public:
		inline OnTriggerExitEvent(Volt::Entity otherEntity)
			: myOtherEntity(otherEntity)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnTriggerExit" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnTriggerExit);
		EVENT_CLASS_CATEGORY(Volt::EventCategoryGraphKey);

		inline const Volt::Entity GetOther() const { return myOtherEntity; }

	private:
		Volt::Entity myOtherEntity;
	};

	class OnTriggerEnterEvent : public Volt::Event
	{
	public:
		inline OnTriggerEnterEvent(Volt::Entity otherEntity)
			: myOtherEntity(otherEntity)
		{
		}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "OnTriggerEnter" << std::endl;
			return ss.str();
		}

		EVENT_CLASS_TYPE(OnTriggerEnter);
		EVENT_CLASS_CATEGORY(Volt::EventCategoryGraphKey);

		inline const Volt::Entity GetOther() const { return myOtherEntity; }

	private:
		Volt::Entity myOtherEntity;
	};

	class OnCollisionEnterNode : public Node
	{
	public:
		OnCollisionEnterNode();
		~OnCollisionEnterNode() override = default;
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "On Collision Enter"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};

	class OnCollisionExitNode : public Node
	{
	public:
		OnCollisionExitNode();
		~OnCollisionExitNode() override = default;
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "On Collision Exit"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};

	class OnTriggerExitNode : public Node
	{
	public:
		OnTriggerExitNode();
		~OnTriggerExitNode() override = default;
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "On Trigger Exit"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};

	class OnTriggerEnterNode : public Node
	{
	public:
		OnTriggerEnterNode();
		~OnTriggerEnterNode() override = default;
		void OnEvent(Volt::Event& e) override;

		inline const std::string GetName() override { return "On Trigger Enter"; }
		inline const gem::vec4 GetColor() override { return { 0.f, 0.f, 1.f, 1.f }; }
	};
}