#pragma once

#include <Volt/Core/UUID.h>
#include <Volt/Events/Event.h>

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
		inline const uint32_t GetNextId() { return myCurrentId++; }
		inline void SetCurrentId(uint32_t id) { myCurrentId = id; }

	private:
		GraphSpecification mySpecification;
		uint32_t myCurrentId = 0;
	};
}