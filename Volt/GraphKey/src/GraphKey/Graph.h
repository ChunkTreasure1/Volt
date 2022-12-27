#pragma once

#include <Volt/Core/UUID.h>
#include <Volt/Events/Event.h>

#include <Volt/Scene/Entity.h>

namespace GraphKey
{
	struct Node;
	struct Link;
	struct Attribute;

	struct GraphSpecification
	{
		std::string name;
		std::filesystem::path path;
		std::vector<Ref<Node>> nodes;
		std::vector<Ref<Link>> links;
	};

	class Graph
	{
	public:
		Graph();
		Graph(Wire::EntityId entity);
		Graph(const GraphSpecification& spec);

		void OnEvent(Volt::Event& e);

		void AddNode(Ref<Node> node);
		void AddLink(Ref<Link> link);

		void CreateLink(const Volt::UUID inputId, const Volt::UUID outputId);

		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

		Attribute* GetAttributeByID(const Volt::UUID id) const;
		
		Ref<Link> GetLinkByID(const Volt::UUID id);
		Ref<Node> GetNodeByID(const Volt::UUID id);

		const bool IsAttributeLinked(const Volt::UUID id) const;

		inline GraphSpecification& GetSpecification() { return mySpecification; }
		inline const Wire::EntityId GetEntity() const { return myEntity; }

		static void Copy(Ref<Graph> srcGraph, Ref<Graph> dstGraph);

	private:
		Wire::EntityId myEntity;
		GraphSpecification mySpecification;
	};
}