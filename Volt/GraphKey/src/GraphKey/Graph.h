#pragma once

namespace GraphKey
{
	struct Node;
	struct Link;

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

		void AddNode(Ref<Node> node);
		void AddLink(Ref<Link> link);

		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

		inline GraphSpecification& GetSpecification() { return mySpecification; }
		inline const uint32_t GetNextId() { return myCurrentId++; }
		inline void SetCurrentId(uint32_t id) { myCurrentId = id; }

	private:
		GraphSpecification mySpecification;
		uint32_t myCurrentId = 0;
	};
}