#pragma once

#include "GraphKey/Link.h"
#include "GraphKey/EventSystem.h"

#include <Volt/Core/UUID.h>
#include <Volt/Events/Event.h>
#include <Volt/Asset/Asset.h>

#include <Volt/Scene/Entity.h>

#include <yaml-cpp/yaml.h>

namespace GraphKey
{
	struct Node;
	struct Attribute;

	struct GraphParameter
	{
		std::string name;
		std::any value;
		Volt::UUID id;
	};

	struct GraphEvent
	{
		std::string name;
		Volt::UUID id;
	};

	class Graph : public Volt::Asset, public std::enable_shared_from_this<Graph>
	{
	public:
		Graph();
		Graph(entt::entity entity);
		~Graph() override = default;

		void OnEvent(Volt::Event& e);

		void AddNode(Ref<Node> node);
		void AddLink(Link link);

		const Volt::UUID CreateLink(const Volt::UUID inputId, const Volt::UUID outputId);
		const Volt::UUID CreateLink(const Volt::UUID linkId, const Volt::UUID inputId, const Volt::UUID outputId);

		void RemoveNode(Volt::UUID id);
		void RemoveLink(Volt::UUID id);

		const std::vector<Ref<Node>> GetNodesOfType(const std::string& type);

		Attribute* GetAttributeByID(const Volt::UUID id) const;
		Link* GetLinkByID(const Volt::UUID id);
		Ref<Node> GetNodeByID(const Volt::UUID id);
		Ref<Node> GetNodeFromAttributeID(const Volt::UUID id);

		const bool IsAttributeLinked(const Volt::UUID id) const;

		inline const entt::entity GetEntity() const { return myEntity; }
		inline std::vector<Ref<Node>>& GetNodes() { return myNodes; }
		inline std::vector<Link>& GetLinks() { return myLinks; }
		inline std::vector<GraphParameter>& GetBlackboard() { return myParentBlackboard ? *myParentBlackboard : myBlackboard; }
		inline std::vector<GraphEvent>& GetEvents() { return myGraphEvents; }
		inline EventSystem& GetEventSystem() { return myEventSystem; }

		inline void SetEntity(entt::entity id) { myEntity = id; }
		inline void SetParentBlackboard(std::vector<GraphParameter>* blackboard) { myParentBlackboard = blackboard; }

		void AddEvent(const std::string& name, const Volt::UUID id = {});
		const std::string GetEventNameFromId(const Volt::UUID id);

		const std::string GetParameterNameFromId(const Volt::UUID id);
		const bool HasParameter(const std::string& name) const;
		const bool HasParameter(const Volt::UUID& id) const;
		void AddParameter(const std::string& name, std::any value, const Volt::UUID id = {});

		template<typename T>
		inline void AddParameter(const std::string& name);

		template<typename T>
		inline const T GetParameterValue(const std::string& name);

		template<typename T>
		inline void SetParameterValue(const std::string& name, const T& value);

		template<typename T>
		inline const T GetParameterValue(const Volt::UUID id);

		template<typename T>
		inline void SetParameterValue(const Volt::UUID id, const T& value);

		void RemoveParameter(const std::string& name);

		static void Copy(Ref<Graph> srcGraph, Ref<Graph> dstGraph);
		static void Serialize(Ref<Graph> graph, YAML::Emitter& out);
		static void Deserialize(Ref<Graph> graph, const YAML::Node& node);

		static Volt::AssetType GetStaticType() { return Volt::AssetType::None; }
		Volt::AssetType GetType() override { return GetStaticType(); };

	private:
		EventSystem myEventSystem;

		std::vector<GraphParameter>* myParentBlackboard = nullptr;
		std::vector<GraphParameter> myBlackboard;
		std::vector<GraphEvent> myGraphEvents;

		entt::entity myEntity = entt::null;

		std::vector<Ref<Node>> myNodes;
		std::vector<Link> myLinks;
		std::string myName;
	};

	template<typename T>
	inline void Graph::AddParameter(const std::string& name)
	{
		std::string paramName = name;

		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		uint32_t counter = 0;
		while (auto it = std::find_if(blackboard.begin(), blackboard.end(), [&paramName](const auto& lhs)
		{
			return lhs.name == paramName;
		}) != blackboard.end())
		{
			paramName = std::format("{0} ({1})", name, counter);
			counter++;
		}

		blackboard.emplace_back(paramName, T{});
	}

	template<typename T>
	inline const T Graph::GetParameterValue(const std::string& name)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&name](const auto& lhs)
		{
			return lhs.name == name;
		});

		if (it == blackboard.end())
		{
			return T{};
		}

		return std::any_cast<T>(it->value);
	}

	template<typename T>
	inline void Graph::SetParameterValue(const std::string& name, const T& value)
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

		it->value = value;
	}

	template<typename T>
	inline const T Graph::GetParameterValue(const Volt::UUID id)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it == blackboard.end())
		{
			return T{};
		}

		return std::any_cast<T>(it->value);
	}

	template<typename T>
	inline void Graph::SetParameterValue(const Volt::UUID id, const T& value)
	{
		auto& blackboard = myParentBlackboard ? *myParentBlackboard : myBlackboard;

		auto it = std::find_if(blackboard.begin(), blackboard.end(), [&id](const auto& lhs)
		{
			return lhs.id == id;
		});

		if (it == blackboard.end())
		{
			return;
		}

		it->value = value;
	}
}
