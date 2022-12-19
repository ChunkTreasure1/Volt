#pragma once

#include "Graph.h"

#include <Volt/Core/UUID.h>
#include <Volt/Core/Base.h>
#include <Volt/Events/Event.h>

#include <Wire/Entity.h>

#include <any>
#include <vector>
#include <string>
#include <functional>

#define GK_BIND_FUNCTION(fn) std::bind(&fn, this)

namespace GraphKey
{
	enum class AttributeType
	{
		Flow,
		Type,
	};

	enum class AttributeDirection
	{
		Input,
		Output,
	};

	struct InputAttribute;
	struct OutputAttribute;

	struct Link
	{
		Volt::UUID input = 0;
		Volt::UUID output = 0;

		Volt::UUID id{};
	};

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
		bool hidden = false;
		
		AttributeType type = AttributeType::Flow;
		AttributeDirection direction;
	};

	struct Node
	{
		Node() = default;
		virtual ~Node() = default;
		Node(const Ref<Node> node);

		virtual void OnEvent(Volt::Event& e) {  }
		
		virtual const std::string GetName() = 0;
		virtual const std::string GetCategory() { return "Default"; }
		virtual const gem::vec4 GetColor() = 0;

		template<typename T>
		Attribute AttributeConfig(const std::string& name, AttributeDirection direction, bool hidden = false, const std::function<void()>& function = nullptr);
		Attribute AttributeConfig(const std::string& name, AttributeDirection direction, const std::function<void()>& function);

		template<typename T>
		const T& GetInput(uint32_t index);

		template<typename T>
		void ActivateOutput(uint32_t index, const T& data);
		void ActivateOutput(uint32_t index);

		template<typename T>
		void SetOutputData(uint32_t index, const T& data);

		Volt::UUID id{};
		Wire::EntityId entity = Wire::NullID;

		std::vector<Attribute> inputs;
		std::vector<Attribute> outputs;

	private:
		friend class Graph;

		Graph* myGraph = nullptr;
	};

	template<typename T>
	inline Attribute Node::AttributeConfig(const std::string& name, AttributeDirection direction, bool linkable, const std::function<void()>& function)
	{
		Attribute attr{};
		attr.name = name;
		attr.direction = direction;
		attr.hidden = linkable;
		attr.function = function;
		attr.data = T();
		attr.type = AttributeType::Type;

		return attr;
	}


	template<typename T>
	inline const T& Node::GetInput(uint32_t index)
	{
		VT_CORE_ASSERT(index < inputs.size(), "Index out of bounds!");

		// If the requested input does not have a value, try to get it from the connected node
		if (!inputs[index].links.empty())
		{
			inputs[index].data.reset();
			
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
					attr->function();
				}
			}
		}
		VT_CORE_ASSERT(inputs[index].data.has_value(), "Input data is empty!");
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
		}
	}
}