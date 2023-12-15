#include "gkpch.h"
#include "Graph.h"

#include "GraphKey/Node.h"
#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/ParameterNodes.h"
#include "GraphKey/Nodes/CustomEventNode.h"

#include <Volt/Asset/Importers/SceneImporter.h>

#include <Volt/Utility/SerializationMacros.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>

inline static std::unordered_map<std::type_index, std::function<void(const std::any& data, YAML::Emitter& out)>> s_typeSerializers;
inline static std::unordered_map<std::type_index, std::function<void(std::any& data, const YAML::Node& node)>> s_typeDeserializers;
inline static bool s_initialized = false;

namespace Volt
{
	template<typename T>
	void RegisterTypeSerializer()
	{
		s_typeSerializers[std::type_index{ typeid(T) }] = [](const std::any& data, YAML::Emitter& out)
		{
			T var = std::any_cast<T>(data);
			VT_SERIALIZE_PROPERTY(data, var, out);
		};

		s_typeDeserializers[std::type_index{ typeid(T) }] = [](std::any& data, const YAML::Node& node)
		{
			T tempData = {};
			VT_DESERIALIZE_PROPERTY(data, tempData, node, T{});

			data = tempData;
		};
	}

	void RegisterEntitySerializer()
	{
		s_typeSerializers[std::type_index{ typeid(Volt::Entity) }] = [](const std::any& data, YAML::Emitter& out)
		{
			Volt::Entity var = std::any_cast<Volt::Entity>(data);
			VT_SERIALIZE_PROPERTY(data, var.GetID(), out);
		};

		s_typeDeserializers[std::type_index{ typeid(Volt::Entity) }] = [](std::any& data, const YAML::Node& node)
		{
			entt::entity tempData = {};
			VT_DESERIALIZE_PROPERTY(data, tempData, node, (entt::entity)entt::null);

			data = Volt::Entity{ tempData, nullptr };
		};
	}
}

namespace GraphKey
{


	Graph::Graph()
	{
		if (!s_initialized)
		{
			s_initialized = true;

			Volt::RegisterTypeSerializer<bool>();
			Volt::RegisterTypeSerializer<int64_t>();
			Volt::RegisterTypeSerializer<uint64_t>();

			Volt::RegisterTypeSerializer<int32_t>();
			Volt::RegisterTypeSerializer<uint32_t>();

			Volt::RegisterTypeSerializer<int16_t>();
			Volt::RegisterTypeSerializer<uint16_t>();

			Volt::RegisterTypeSerializer<int8_t>();
			Volt::RegisterTypeSerializer<uint8_t>();

			Volt::RegisterTypeSerializer<float>();
			Volt::RegisterTypeSerializer<double>();

			Volt::RegisterTypeSerializer<glm::vec2>();
			Volt::RegisterTypeSerializer<glm::vec3>();
			Volt::RegisterTypeSerializer<glm::vec4>();

			Volt::RegisterTypeSerializer<glm::quat>();

			Volt::RegisterTypeSerializer<std::string>();
			Volt::RegisterTypeSerializer<Volt::AssetHandle>();

			Volt::RegisterEntitySerializer();
		}
	}

	Graph::Graph(Volt::EntityID entity)
		: myEntity(entity)
	{
	}

	void Graph::OnEvent(Volt::Event& e)
	{
		for (const auto& n : myNodes)
		{
			n->OnEvent(e);
		}
	}

	void Graph::AddNode(Ref<Node> node)
	{
		node->myGraph = this;
		myNodes.emplace_back(node);
		node->Initialize();
	}

	void Graph::AddLink(Link link)
	{
		myLinks.emplace_back(link);
	}

	const Volt::UUID Graph::CreateLink(const Volt::UUID inputId, const Volt::UUID outputId)
	{
		return CreateLink(Volt::UUID{}, inputId, outputId);
	}

	const Volt::UUID Graph::CreateLink(const Volt::UUID linkId, const Volt::UUID inputId, const Volt::UUID outputId)
	{
		Link newLink{};
		newLink.id = linkId;
		newLink.input = outputId;
		newLink.output = inputId;

		myLinks.emplace_back(newLink);

		Attribute* input = GetAttributeByID(inputId);
		Attribute* output = GetAttributeByID(outputId);

		if (input)
		{
			input->links.emplace_back(newLink.id);
		}

		if (output)
		{
			while (!output->links.empty())
			{
				RemoveLink(output->links.front());
			}
			output->links.emplace_back(newLink.id);
		}

		return newLink.id;
	}

	void Graph::RemoveNode(Volt::UUID id)
	{
		auto it = std::find_if(myNodes.begin(), myNodes.end(), [&id](const auto& lhs)
		{
			return lhs->id == id;
		});

		if (it == myNodes.end())
		{
			return;
		}

		Ref<Node> node = *it;
		for (const auto& attr : node->inputs)
		{
			for (const auto& l : attr.links)
			{
				RemoveLink(l);
			}
		}

		for (const auto& attr : node->outputs)
		{
			for (const auto& l : attr.links)
			{
				RemoveLink(l);
			}
		}

		myNodes.erase(it);
	}

	void Graph::RemoveLink(Volt::UUID id)
	{
		auto it = std::find_if(myLinks.begin(), myLinks.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it == myLinks.end())
		{
			return;
		}

		auto link = *it;

		Attribute* input = GetAttributeByID(link.input);
		Attribute* output = GetAttributeByID(link.output);

		input->links.erase(std::remove(input->links.begin(), input->links.end(), link.id), input->links.end());
		output->links.erase(std::remove(output->links.begin(), output->links.end(), link.id), output->links.end());

		myLinks.erase(it);
	}

	Attribute* Graph::GetAttributeByID(const Volt::UUID id) const
	{
		for (const auto& node : myNodes)
		{
			for (auto& input : node->inputs)
			{
				if (input.id == id)
				{
					return &input;
				}
			}

			for (auto& output : node->outputs)
			{
				if (output.id == id)
				{
					return &output;
				}
			}
		}

		return nullptr;
	}

	Link* Graph::GetLinkByID(const Volt::UUID id)
	{
		if (auto it = std::find_if(myLinks.begin(), myLinks.end(), [&](const Link& link) { return link.id == id; }); it != myLinks.end())
		{
			return &(*it);
		}

		return nullptr;
	}

	Ref<Node> Graph::GetNodeByID(const Volt::UUID id)
	{
		if (auto it = std::find_if(myNodes.begin(), myNodes.end(), [&](const Ref<Node>& node) { return node->id == id; }); it != myNodes.end())
		{
			return *it;
		}

		return nullptr;
	}

	Ref<Node> Graph::GetNodeFromAttributeID(const Volt::UUID id)
	{
		for (const auto& n : myNodes)
		{
			for (const auto& i : n->inputs)
			{
				if (i.id == id)
				{
					return n;
				}
			}

			for (const auto& o : n->outputs)
			{
				if (o.id == id)
				{
					return n;
				}
			}
		}

		return nullptr;
	}

	const bool Graph::IsAttributeLinked(const Volt::UUID id) const
	{
		const auto* attr = GetAttributeByID(id);
		if (attr)
		{
			return !attr->links.empty();
		}

		return false;
	}

	const std::vector<Ref<Node>> Graph::GetNodesOfType(const std::string& type)
	{
		std::vector<Ref<Node>> result{};

		for (const auto& n : myNodes)
		{
			if (n->GetRegistryName() == type)
			{
				result.emplace_back(n);
			}
		}

		return result;
	}

	void Graph::Copy(Ref<Graph> srcGraph, Ref<Graph> dstGraph)
	{
		for (const auto& n : srcGraph->myNodes)
		{
			Ref<GraphKey::Node> newNode;

			if (dstGraph->GetEntity() != Volt::Entity::NullID())
			{
				newNode = n->CreateCopy(dstGraph.get(), dstGraph->GetEntity());
			}
			else
			{
				newNode = n->CreateCopy(dstGraph.get());
			}
			dstGraph->myNodes.emplace_back(newNode);
			newNode->OnCopy();
		}

		for (const auto& l : srcGraph->myLinks)
		{
			Link& newLink = dstGraph->myLinks.emplace_back();
			newLink.id = l.id;
			newLink.input = l.input;
			newLink.output = l.output;
		}

		if (dstGraph->myEntity == Volt::Entity::NullID())
		{
			dstGraph->myEntity = srcGraph->myEntity;
		}

		dstGraph->myBlackboard = srcGraph->myBlackboard;
		dstGraph->myGraphEvents = srcGraph->myGraphEvents;
	}

	void Graph::Serialize(Ref<Graph> graph, YAML::Emitter& out)
	{
		out << YAML::Key << "Graph" << YAML::Value;
		{
			out << YAML::BeginMap;
			{
				out << YAML::Key << "Nodes" << YAML::BeginSeq;
				for (const auto& n : graph->GetNodes())
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(id, n->id, out);
						VT_SERIALIZE_PROPERTY(type, n->GetRegistryName(), out);
						VT_SERIALIZE_PROPERTY(state, n->editorState, out);

						out << YAML::Key << "nodeSpecific" << YAML::BeginMap;
						n->Serialize(out);
						out << YAML::EndMap;

						if (auto paramType = std::dynamic_pointer_cast<GraphKey::ParameterNode>(n))
						{
							VT_SERIALIZE_PROPERTY(parameterId, paramType->parameterId, out);
						}
						else if (auto eventType = std::dynamic_pointer_cast<GraphKey::CustomEventNode>(n))
						{
							VT_SERIALIZE_PROPERTY(eventId, eventType->eventId, out);
						}

						out << YAML::Key << "inputs" << YAML::BeginSeq;
						{
							for (const auto& i : n->inputs)
							{
								out << YAML::BeginMap;
								{
									VT_SERIALIZE_PROPERTY(name, i.name, out);
									VT_SERIALIZE_PROPERTY(id, i.id, out);

									const bool shouldSerialize = !graph->IsAttributeLinked(i.id) && i.data.has_value();
									if (shouldSerialize && s_typeSerializers.contains(i.data.type()))
									{
										s_typeSerializers[i.data.type()](i.data, out);
									}
								}
								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq;

						out << YAML::Key << "outputs" << YAML::BeginSeq;
						{
							for (const auto& o : n->outputs)
							{
								out << YAML::BeginMap;
								{
									VT_SERIALIZE_PROPERTY(name, o.name, out);
									VT_SERIALIZE_PROPERTY(id, o.id, out);

									const bool shouldSerialize = o.data.has_value();
									if (shouldSerialize && s_typeSerializers.contains(o.data.type()))
									{
										s_typeSerializers[o.data.type()](o.data, out);
									}
								}
								out << YAML::EndMap;
							}
							out << YAML::EndSeq;
						}
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Links" << YAML::BeginSeq;
				for (const auto& l : graph->GetLinks())
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(id, l.id, out);
						VT_SERIALIZE_PROPERTY(output, l.output, out);
						VT_SERIALIZE_PROPERTY(input, l.input, out);
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Parameters" << YAML::BeginSeq;
				for (const auto& p : graph->GetBlackboard())
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(name, p.name, out);
						VT_SERIALIZE_PROPERTY(type, GraphKey::TypeRegistry::GetNameFromTypeIndex(p.value.type()), out);
						VT_SERIALIZE_PROPERTY(id, p.id, out);

						if (s_typeSerializers.contains(p.value.type()))
						{
							s_typeSerializers[p.value.type()](p.value, out);
						}
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Events" << YAML::BeginSeq;
				for (const auto& e : graph->GetEvents())
				{
					out << YAML::BeginMap;
					{
						VT_SERIALIZE_PROPERTY(name, e.name, out);
						VT_SERIALIZE_PROPERTY(id, e.id, out);
					}
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}
			out << YAML::EndMap;
		}
	}

	void Graph::Deserialize(Ref<Graph> graph, const YAML::Node& yamlNode)
	{
		struct Attribute
		{
			std::string name;
			Volt::UUID id;

			YAML::Node node;
		};

		struct NodeData
		{
			Volt::UUID id;
			Volt::UUID parameterId;
			Volt::UUID eventId;

			std::string type;
			std::string state;

			YAML::Node nodeSpecific;

			std::vector<Attribute> inputs;
			std::vector<Attribute> outputs;
		};

		struct LinkData
		{
			Volt::UUID id;

			Volt::UUID input;
			Volt::UUID output;
		};

		struct Parameter
		{
			std::string name;
			std::any value;
			Volt::UUID id;
		};

		struct Event
		{
			std::string name;
			Volt::UUID id;
		};

		std::vector<NodeData> nodes;
		std::vector<LinkData> links;
		std::vector<Parameter> parameters;
		std::vector<Event> events;

		for (const auto& n : yamlNode["Nodes"])
		{
			auto& data = nodes.emplace_back();
			VT_DESERIALIZE_PROPERTY(id, data.id, n, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(type, data.type, n, std::string(""));
			VT_DESERIALIZE_PROPERTY(parameterId, data.parameterId, n, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(eventId, data.eventId, n, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(state, data.state, n, std::string(""));

			if (n["nodeSpecific"])
			{
				data.nodeSpecific.reset(n["nodeSpecific"]);
			}

			for (const auto& i : n["inputs"])
			{
				auto& inData = data.inputs.emplace_back();
				VT_DESERIALIZE_PROPERTY(id, inData.id, i, Volt::UUID(0));
				VT_DESERIALIZE_PROPERTY(name, inData.name, i, std::string("Null"));

				if (i["data"])
				{
					inData.node.reset(i);
				}
			}

			for (const auto& o : n["outputs"])
			{
				auto& outData = data.outputs.emplace_back();
				VT_DESERIALIZE_PROPERTY(id, outData.id, o, Volt::UUID(0));
				VT_DESERIALIZE_PROPERTY(name, outData.name, o, std::string("Null"));

				if (o["data"])
				{
					outData.node.reset(o);
				}
			}
		}

		for (const auto& l : yamlNode["Links"])
		{
			auto& data = links.emplace_back();
			VT_DESERIALIZE_PROPERTY(id, data.id, l, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(input, data.input, l, Volt::UUID(0));
			VT_DESERIALIZE_PROPERTY(output, data.output, l, Volt::UUID(0));
		}

		for (const auto& p : yamlNode["Parameters"])
		{
			auto& data = parameters.emplace_back();
			VT_DESERIALIZE_PROPERTY(name, data.name, p, std::string(""));
			VT_DESERIALIZE_PROPERTY(id, data.id, p, Volt::UUID(0));

			std::string type;
			VT_DESERIALIZE_PROPERTY(type, type, p, std::string(""));
			if (type.empty())
			{
				continue;
			}

			data.value = GraphKey::TypeRegistry::GetDefaultValueFromName(type);

			if (s_typeDeserializers.contains(data.value.type()))
			{
				s_typeDeserializers[data.value.type()](data.value, p);
			}
		}

		for (const auto& e : yamlNode["Events"])
		{
			auto& data = events.emplace_back();
			VT_DESERIALIZE_PROPERTY(name, data.name, e, std::string(""));
			VT_DESERIALIZE_PROPERTY(id, data.id, e, Volt::UUID(0));
		}

		for (const auto& p : parameters)
		{
			graph->AddParameter(p.name, p.value, p.id);
		}

		for (const auto& e : events)
		{
			graph->AddEvent(e.name, e.id);
		}

		for (const auto& n : nodes)
		{
			auto node = GraphKey::Registry::Create(n.type);
			node->id = n.id;
			node->myGraph = graph.get();

			node->editorState = n.state;
			node->Initialize();

			node->Deserialize(n.nodeSpecific);

			if (auto paramType = std::dynamic_pointer_cast<GraphKey::ParameterNode>(node))
			{
				paramType->parameterId = n.parameterId;
			}
			else if (auto eventType = std::dynamic_pointer_cast<GraphKey::CustomEventNode>(node))
			{
				eventType->eventId = n.eventId;
			}

			for (const auto& i : n.inputs)
			{
				auto it = std::find_if(node->inputs.begin(), node->inputs.end(), [&i](const auto& lhs)
				{
					return lhs.name == i.name;
				});

				if (it != node->inputs.end())
				{
					it->id = i.id;

					if (i.node && it->data.has_value() && s_typeDeserializers.contains(it->data.type()))
					{
						s_typeDeserializers[it->data.type()](it->data, i.node);
					}
				}
			}

			for (const auto& o : n.outputs)
			{
				auto it = std::find_if(node->outputs.begin(), node->outputs.end(), [&o](const auto& lhs)
				{
					return lhs.name == o.name;
				});

				if (it != node->outputs.end())
				{
					it->id = o.id;

					if (o.node && it->data.has_value() && s_typeDeserializers.contains(it->data.type()))
					{
						s_typeDeserializers[it->data.type()](it->data, o.node);
					}
				}
			}

			graph->GetNodes().emplace_back(node);
		}

		for (const auto& l : links)
		{
			GraphKey::Link newLink{};
			newLink.id = l.id;
			newLink.input = l.input;
			newLink.output = l.output;

			auto inAttr = graph->GetAttributeByID(l.input);
			inAttr->links.emplace_back(l.id);

			auto outAttr = graph->GetAttributeByID(l.output);
			outAttr->links.emplace_back(l.id);

			graph->GetLinks().emplace_back(newLink);
		}
	}

	void Graph::AddEvent(const std::string& name, const Volt::UUID id)
	{
		std::string eventName = name;

		uint32_t counter = 0;
		while (auto it = std::find_if(myGraphEvents.begin(), myGraphEvents.end(), [&eventName](const auto& lhs)
		{
			return lhs.name == eventName;
		}) != myGraphEvents.end())
		{
			eventName = std::format("{0} ({1})", name, counter);
			counter++;
		}

		myGraphEvents.emplace_back(eventName, id);
	}

	const std::string Graph::GetEventNameFromId(const Volt::UUID id)
	{
		auto it = std::find_if(myGraphEvents.begin(), myGraphEvents.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it == myGraphEvents.end())
		{
			return {};
		}

		return (*it).name;
	}

	const std::string Graph::GetParameterNameFromId(const Volt::UUID id)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it == blackboard.end())
		{
			return {};
		}

		return (*it).name;
	}

	const bool Graph::HasParameter(const std::string& name) const
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&name](const auto& lhs)
		{
			return lhs.name == name;
		});

		return it != blackboard.end();
	}

	const bool Graph::HasParameter(const Volt::UUID& id) const
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it != blackboard.end())
		{
			return true;
		}

		return false;
	}

	void Graph::AddParameter(const std::string& name, std::any value, const Volt::UUID id)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;
		blackboard.emplace_back(name, value, id);
	}

	void Graph::RemoveParameter(const std::string& name)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&name](const auto& lhs)
		{
			return lhs.name == name;
		});

		if (it == blackboard.end())
		{
			return;
		}

		blackboard.erase(it);
	}
}
