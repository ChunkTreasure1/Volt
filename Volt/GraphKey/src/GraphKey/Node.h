#pragma once

#include "Graph.h"
#include "GraphKey/Link.h"

#include <Volt/Core/UUID.h>
#include <Volt/Core/Base.h>
#include <Volt/Events/Event.h>

#include <any>
#include <vector>
#include <string>
#include <functional>

#include <entt.hpp>

#include <yaml-cpp/yaml.h>

#define GK_BIND_FUNCTION(fn) std::bind(&fn, this)

namespace GraphKey
{
	enum class AttributeType
	{
		Flow,
		Type,
		AnimationPose,
	};

	enum class AttributeDirection
	{
		Input,
		Output,
	};

	struct InputAttribute;
	struct OutputAttribute;

	struct Attribute
	{
	public:
		Attribute() = default;
		virtual ~Attribute() = default;

		std::string name;
		Volt::UUID id{};

		std::vector<Volt::UUID> links;
		std::function<void()> function;

		std::any data;
		bool inputHidden = false;
		bool linkable = true;

		AttributeType type = AttributeType::Flow;
		AttributeDirection direction;
	};

	struct Node
	{
		Node() = default;
		virtual ~Node() = default;
		Node(const Node& node) = delete;

		virtual void OnEvent(Volt::Event&) {}
		virtual void Initialize() {}
		virtual void OnCopy() {}

		virtual void Serialize(YAML::Emitter&) {}
		virtual void Deserialize(const YAML::Node&) {}
		virtual Ref<Node> CreateCopy(Graph* ownerGraph, Volt::EntityID entity = Volt::Entity::NullID());

		virtual const std::string GetName() = 0;
		virtual const glm::vec4 GetColor() = 0;

		inline const std::string& GetRegistryName() const { return myRegistryName; }
		const uint32_t GetAttributeIndexFromID(const Volt::UUID id) const;

		template<typename T>
		Attribute AttributeConfig(const std::string& name, AttributeDirection direction, bool hidden = false, const std::function<void()>& function = nullptr, bool linkable = true);

		template<typename T>
		Attribute AttributeConfigDefault(const std::string& name, AttributeDirection direction, const T& defaultValue, bool hidden = false, const std::function<void()>& function = nullptr, bool linkable = true);

		Attribute AttributeConfig(const std::string& name, AttributeDirection direction, const std::function<void()>& function = nullptr);

		template<typename T>
		Attribute AttributeConfigAnimationPose(const std::string& name, AttributeDirection direction, const std::function<void()>& function = nullptr);

		template<typename T>
		const T& GetInput(uint32_t index);

		const bool InputHasData(uint32_t index);

		template<typename T>
		void ActivateOutput(uint32_t index, const T& data);
		void ActivateOutput(uint32_t index);

		template<typename T>
		void SetOutputData(uint32_t index, const T& data);

		Volt::UUID id{};
		Volt::EntityID nodeEntity = Volt::Entity::NullID();

		std::vector<Attribute> inputs;
		std::vector<Attribute> outputs;

		std::string editorState;
		bool isHeaderless = false;

	protected:
		Graph* myGraph = nullptr;

	private:
		friend class Graph;
		friend class Registry;

		std::string myRegistryName;
	};

	template<typename T>
	inline Attribute Node::AttributeConfig(const std::string& name, AttributeDirection direction, bool hidden, const std::function<void()>& function, bool linkable)
	{
		Attribute attr{};
		attr.name = name;
		attr.direction = direction;
		attr.inputHidden = hidden;
		attr.linkable = linkable;
		attr.function = function;
		attr.data = T();
		attr.type = AttributeType::Type;

		return attr;
	}

	template<typename T>
	inline Attribute Node::AttributeConfigDefault(const std::string& name, AttributeDirection direction, const T& defaultValue, bool hidden, const std::function<void()>& function, bool linkable)
	{
		Attribute attr{};
		attr.name = name;
		attr.direction = direction;
		attr.inputHidden = hidden;
		attr.linkable = linkable;
		attr.function = function;
		attr.data = defaultValue;
		attr.type = AttributeType::Type;

		return attr;
	}


	template<typename T>
	inline Attribute Node::AttributeConfigAnimationPose(const std::string& name, AttributeDirection direction, const std::function<void()>& function)
	{
		Attribute attr{};
		attr.name = name;
		attr.direction = direction;
		attr.inputHidden = false;
		attr.linkable = true;
		attr.function = function;
		attr.data = T();
		attr.type = AttributeType::AnimationPose;
		
		return attr;
	}

	template<typename T>
	inline const T& Node::GetInput(uint32_t index)
	{
		VT_CORE_ASSERT(index < inputs.size(), "Index out of bounds!");

		// If the requested input does not have a value, try to get it from the connected node
		if (!inputs[index].links.empty())
		{
			for (const auto& linkId : inputs[index].links)
			{
				const auto link = myGraph->GetLinkByID(linkId);
				if (!link)
				{
					continue;
				}

				const auto attr = myGraph->GetAttributeByID(link->output);
				if (attr)
				{
					if (attr->function)
					{
						inputs[index].data.reset();
						attr->function();
					}
				}
			}
		}

		if (inputs[index].data.type() != typeid(T))
		{
			static T invalidTypeValue{};
			return invalidTypeValue;
		}

		return std::any_cast<const T&>(inputs[index].data);
	}

	template<typename T>
	inline void Node::ActivateOutput(uint32_t index, const T& data)
	{
		for (const auto& linkId : outputs[index].links)
		{
			const auto link = myGraph->GetLinkByID(linkId);
			if (!link)
			{
				continue;
			}

			const auto attr = myGraph->GetAttributeByID(link->input);
			if (!attr)
			{
				continue;
			}

			attr->data = data;
			if (attr->function)
			{
				attr->function();
			}
		}
	}

	template<typename T>
	inline void Node::SetOutputData(uint32_t index, const T& data)
	{
		VT_CORE_ASSERT(index < outputs.size(), "Index out of bounds!");

		outputs[index].data = data;

		for (const auto& linkId : outputs[index].links)
		{
			const auto link = myGraph->GetLinkByID(linkId);
			if (!link)
			{
				continue;
			}

			const auto inAttr = myGraph->GetAttributeByID(link->input);

			if (!inAttr)
			{
				continue;
			}

			inAttr->data = data;
		}
	}
}
