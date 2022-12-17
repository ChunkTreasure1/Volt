#pragma once

#include <Wire/Entity.h>
#include <any>

namespace GraphKey
{
	struct InputAttribute;
	struct OutputAttribute;

	struct Link
	{
		InputAttribute* input = nullptr;
		OutputAttribute* output = nullptr;

		uint32_t id;
	};

	struct Attribute
	{
	public:
		Attribute() = default;
		virtual ~Attribute() = default;

		std::string name;
		uint32_t id;

		std::vector<Link*> links;
		std::any data;
		bool linkable = true;
	};

	struct InputAttribute : public Attribute
	{
		InputAttribute(const std::string& name, bool linkable);
		std::function<void()> function = nullptr;
	};

	struct OutputAttribute : public Attribute
	{
		OutputAttribute(const std::string& name, bool linkable);
		std::function<void()> function = nullptr;
	};

	struct Node
	{
		Node() = default;
		Node(const Ref<Node> node);

		template<typename T>
		InputAttribute InputAttributeConfig(const std::string& name, bool linkable = true, const std::function<void()>& function = nullptr);

		template<typename T>
		OutputAttribute OutputAttributeConfig(const std::string& name, bool linkable = true, const std::function<void()>& function = nullptr);

		template<typename T>
		const T& GetInput(uint32_t index);

		template<typename T>
		void ActivateOutput(uint32_t index, const T& data);

		uint32_t id = 0;
		Wire::EntityId entity = Wire::NullID;

		std::string name;
		std::vector<Ref<Link>> links;
		std::vector<InputAttribute> inputs;
		std::vector<OutputAttribute> outputs;
	};

	template<typename T>
	inline InputAttribute Node::InputAttributeConfig(const std::string& name, bool linkable, const std::function<void()>& function)
	{
		InputAttribute attr{};
		attr.name = name;
		attr.linkable = linkable;
		attr.function = function;
		attr.data = T();

		return attr;
	}

	template<typename T>
	inline OutputAttribute Node::OutputAttributeConfig(const std::string& name, bool linkable, const std::function<void()>& function)
	{
		OutputAttribute attr{};
		attr.name = name;
		attr.linkable = linkable;
		attr.function = function;
		attr.data = T();

		return attr;
	}

	template<typename T>
	inline const T& Node::GetInput(uint32_t index)
	{
		VT_CORE_ASSERT(index < inputs.size(), "Index out of bounds!");

		// If the requested input does not have a value, try to get it from the connected node
		if (!inputs[index].data.has_value() && !inputs[index].links.empty())
		{
			for (const auto& link : inputs[index].links)
			{
				if (link->output->function)
				{
					link->output->function();
				}
			}
			VT_CORE_ASSERT(inputs[index].data.has_value(), "Input data is empty!");
		}

		return std::any_cast<const T&>(inputs[index].data);
	}

	template<typename T>
	inline void Node::ActivateOutput(uint32_t index, const T& data)
	{
		for (const auto& link : outputs[index].links)
		{
			link->input->data = data;
			if (link->input->function != nullptr)
			{
				link->input->function();
			}
		}
	}
}