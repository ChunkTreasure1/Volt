#include "gkpch.h"
#include "Graph.h"

#include "GraphKey/Node.h"
#include "GraphKey/Registry.h"

#include "GraphKey/Nodes/ParameterNodes.h"
#include "GraphKey/Nodes/CustomEventNode.h"

#include <Volt/Asset/Importers/SceneImporter.h>
#include <Volt/Utility/YAMLSerializationHelpers.h>

#include <CoreUtilities/FileIO/YAMLStreamReader.h>
#include <CoreUtilities/FileIO/YAMLStreamWriter.h>

inline static std::unordered_map<std::type_index, std::function<void(const std::any& data, YAMLStreamWriter& out)>> s_typeSerializers;
inline static std::unordered_map<std::type_index, std::function<void(std::any& data, YAMLStreamReader& node)>> s_typeDeserializers;
inline static bool s_initialized = false;

namespace Volt
{
	template<typename T>
	void RegisterTypeSerializer()
	{
		s_typeSerializers[std::type_index{ typeid(T) }] = [](const std::any& data, YAMLStreamWriter& out)
		{
			T var = std::any_cast<T>(data);
			out.SetKey("data", var);
		};

		s_typeDeserializers[std::type_index{ typeid(T) }] = [](std::any& data, YAMLStreamReader& node)
		{
			T tempData = node.ReadAtKey("data", T{});
			data = tempData;
		};
	}

	void RegisterEntitySerializer()
	{
		s_typeSerializers[std::type_index{ typeid(Volt::Entity) }] = [](const std::any& data, YAMLStreamWriter& out)
		{
			Volt::Entity var = std::any_cast<Volt::Entity>(data);
			out.SetKey("data", var.GetID());
		};

		s_typeDeserializers[std::type_index{ typeid(Volt::Entity) }] = [](std::any& data, YAMLStreamReader& node)
		{
			entt::entity tempData = node.ReadAtKey("data", (entt::entity)entt::null);
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

	const UUID64 Graph::CreateLink(const UUID64 inputId, const UUID64 outputId)
	{
		return CreateLink(UUID64{}, inputId, outputId);
	}

	const UUID64 Graph::CreateLink(const UUID64 linkId, const UUID64 inputId, const UUID64 outputId)
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

	void Graph::RemoveNode(UUID64 id)
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

	void Graph::RemoveLink(UUID64 id)
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

	Attribute* Graph::GetAttributeByID(const UUID64 id) const
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

	Link* Graph::GetLinkByID(const UUID64 id)
	{
		if (auto it = std::find_if(myLinks.begin(), myLinks.end(), [&](const Link& link) { return link.id == id; }); it != myLinks.end())
		{
			return &(*it);
		}

		return nullptr;
	}

	Ref<Node> Graph::GetNodeByID(const UUID64 id)
	{
		if (auto it = std::find_if(myNodes.begin(), myNodes.end(), [&](const Ref<Node>& node) { return node->id == id; }); it != myNodes.end())
		{
			return *it;
		}

		return nullptr;
	}

	Ref<Node> Graph::GetNodeFromAttributeID(const UUID64 id)
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

	const bool Graph::IsAttributeLinked(const UUID64 id) const
	{
		const auto* attr = GetAttributeByID(id);
		if (attr)
		{
			return !attr->links.empty();
		}

		return false;
	}

	const Vector<Ref<Node>> Graph::GetNodesOfType(const std::string& type)
	{
		Vector<Ref<Node>> result{};

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

	void Graph::Serialize(Ref<Graph> graph, YAMLStreamWriter& out)
	{
		out.BeginMapNamned("Graph");
		out.BeginSequence("Nodes");
		for (const auto& n : graph->GetNodes())
		{
			out.BeginMap();
			out.SetKey("id", n->id);
			out.SetKey("type", n->GetRegistryName());
			out.SetKey("state", n->editorState);

			out.BeginMapNamned("nodeSpecific");
			n->Serialize(out);
			out.EndMap();

			if (auto paramType = std::dynamic_pointer_cast<GraphKey::ParameterNode>(n))
			{
				out.SetKey("parameterId", paramType->parameterId);
			}
			else if (auto eventType = std::dynamic_pointer_cast<GraphKey::CustomEventNode>(n))
			{
				out.SetKey("eventId", eventType->eventId);
			}

			out.BeginSequence("inputs");
			for (const auto& i : n->inputs)
			{
				out.BeginMap();
				out.SetKey("name", i.name);
				out.SetKey("id", i.id);

				const bool shouldSerialize = !graph->IsAttributeLinked(i.id) && i.data.has_value();
				if (shouldSerialize && s_typeSerializers.contains(i.data.type()))
				{
					s_typeSerializers[i.data.type()](i.data, out);
				}
				out.EndMap();
			}
			out.EndSequence();

			out.BeginSequence("outputs");
			for (const auto& o : n->outputs)
			{
				out.BeginMap();
				out.SetKey("name", o.name);
				out.SetKey("id", o.id);

				const bool shouldSerialize = o.data.has_value();
				if (shouldSerialize && s_typeSerializers.contains(o.data.type()))
				{
					s_typeSerializers[o.data.type()](o.data, out);
				}
				out.EndMap();
			}
			out.EndSequence();

			out.EndMap();
		}
		out.EndSequence();

		out.BeginSequence("Links");
		for (const auto& l : graph->GetLinks())
		{
			out.BeginMap();
			out.SetKey("id", l.id);
			out.SetKey("output", l.output);
			out.SetKey("input", l.input);
			out.EndMap();
		}
		out.EndSequence();

		out.BeginSequence("Parameters");
		for (const auto& p : graph->GetBlackboard())
		{
			out.BeginMap();
			out.SetKey("name", p.name);
			out.SetKey("type", GraphKey::TypeRegistry::GetNameFromTypeIndex(p.value.type()));
			out.SetKey("id", p.id);

			if (s_typeSerializers.contains(p.value.type()))
			{
				s_typeSerializers[p.value.type()](p.value, out);
			}

			out.EndMap();
		}
		out.EndSequence();

		out.BeginSequence("Events");
		for (const auto& e : graph->GetEvents())
		{
			out.BeginMap();
			out.SetKey("name", e.name);
			out.SetKey("id", e.id);
			out.EndMap();
		}
		out.EndSequence();
		out.EndMap();
	}

	void Graph::Deserialize(Ref<Graph> graph, YAMLStreamReader& streamReader)
	{
		struct Attribute
		{
			std::string name;
			UUID64 id;

			YAML::Node node;
		};

		struct NodeData
		{
			UUID64 id;
			UUID64 parameterId;
			UUID64 eventId;

			std::string type;
			std::string state;

			YAML::Node nodeSpecific;

			Vector<Attribute> inputs;
			Vector<Attribute> outputs;
		};

		struct LinkData
		{
			UUID64 id;

			UUID64 input;
			UUID64 output;
		};

		struct Parameter
		{
			std::string name;
			std::any value;
			UUID64 id;
		};

		struct Event
		{
			std::string name;
			UUID64 id;
		};

		Vector<NodeData> nodes;
		Vector<LinkData> links;
		Vector<Parameter> parameters;
		Vector<Event> events;

		streamReader.ForEach("Nodes", [&]()
		{
			auto& data = nodes.emplace_back();
			data.id = streamReader.ReadAtKey("id", UUID64(0));
			data.type = streamReader.ReadAtKey("type", std::string());
			data.parameterId = streamReader.ReadAtKey("parameterId", UUID64(0));
			data.eventId = streamReader.ReadAtKey("eventId", UUID64(0));
			data.state = streamReader.ReadAtKey("state", std::string());

			if (streamReader.HasKey("nodeSpecific"))
			{
				data.nodeSpecific.reset(streamReader.GetRawNode());
			}

			streamReader.ForEach("inputs", [&]()
			{
				auto& inData = data.inputs.emplace_back();
				inData.id = streamReader.ReadAtKey("id", UUID64(0));
				inData.name = streamReader.ReadAtKey("name", std::string("Null"));
			
				if (streamReader.HasKey("data"))
				{
					inData.node.reset(streamReader.GetRawNode());
				}
			});

			streamReader.ForEach("outputs", [&]()
			{
				auto& outData = data.outputs.emplace_back();
				outData.id = streamReader.ReadAtKey("id", UUID64(0));
				outData.name = streamReader.ReadAtKey("name", std::string("Null"));

				if (streamReader.HasKey("data"))
				{
					outData.node.reset(streamReader.GetRawNode());
				}
			});
		});

		streamReader.ForEach("Links", [&]() 
		{
			auto& data = links.emplace_back();
			data.id = streamReader.ReadAtKey("id", UUID64(0));
			data.input = streamReader.ReadAtKey("input", UUID64(0));
			data.output = streamReader.ReadAtKey("output", UUID64(0));
		});

		streamReader.ForEach("Parameters", [&]() 
		{
			auto& data = parameters.emplace_back();
			data.name = streamReader.ReadAtKey("name", std::string());
			data.id = streamReader.ReadAtKey("id", UUID64(0));

			std::string type = streamReader.ReadAtKey("type", std::string());
			if (type.empty())
			{
				return;
			}

			data.value = GraphKey::TypeRegistry::GetDefaultValueFromName(type);
			if (s_typeDeserializers.contains(data.value.type()))
			{
				s_typeDeserializers[data.value.type()](data.value, streamReader);
			}
		});

		streamReader.ForEach("Events", [&]() 
		{
			auto& data = events.emplace_back();
			data.name = streamReader.ReadAtKey("name", std::string());
			data.id = streamReader.ReadAtKey("id", UUID64(0));
		});

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

			//node->Deserialize(n.nodeSpecific);

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

					// #TODO_Ivar: Fix
					//if (i.node && it->data.has_value() && s_typeDeserializers.contains(it->data.type()))
					//{
					//	s_typeDeserializers[it->data.type()](it->data, i.node);
					//}
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

					//if (o.node && it->data.has_value() && s_typeDeserializers.contains(it->data.type()))
					//{
					//	s_typeDeserializers[it->data.type()](it->data, o.node);
					//}
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

	void Graph::AddEvent(const std::string& name, const UUID64 id)
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

	const std::string Graph::GetEventNameFromId(const UUID64 id)
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

	const std::string Graph::GetParameterNameFromId(const UUID64 id)
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

	const bool Graph::HasParameter(const UUID64& id) const
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

	void Graph::AddParameter(const std::string& name, std::any value, const UUID64 id)
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
