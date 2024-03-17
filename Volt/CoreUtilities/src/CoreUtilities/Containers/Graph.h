#pragma once

#include "CoreUtilities/UUID.h"

#include <vector>
#include <unordered_map>
#include <ranges>

template<typename NodeDataType, typename EdgeMetadataType>
class Graph;

template<typename MetaDataType>
struct GraphEdge
{
	UUID64 id = 0;
	UUID64 startNode = 0;
	UUID64 endNode = 0;

	MetaDataType metaDataType{};

	inline const bool IsValid() const { return id != 0; }
};

template<typename NodeDataType, typename EdgeMetadataType>
struct GraphNode
{
	GraphNode(UUID64 id, NodeDataType data, Graph<NodeDataType, EdgeMetadataType>* graph)
		: id(id), nodeData(data), m_graph(graph)
	{}

	UUID64 id = 0;
	NodeDataType nodeData{};

	std::vector<UUID64> edges;

	const std::vector<UUID64> GetInputEdges() const;
	const std::vector<UUID64> GetOutputEdges() const;

	const GraphEdge<EdgeMetadataType>& GetEdgeFromID(const UUID64 edgeId) const;
	const GraphNode<NodeDataType, EdgeMetadataType>& GetNodeFromID(const UUID64 nodeId) const;

	void RemoveEdge(const UUID64 edgeId);

	inline const bool IsValid() const { return id != 0; }

private:
	Graph<NodeDataType, EdgeMetadataType>* m_graph = nullptr;
};

template<typename NodeDataType, typename EdgeMetadataType>
class Graph
{
public:
	Graph() {}
	~Graph() {}

	const UUID64 AddNode(const UUID64 nodeId, const NodeDataType& data);
	const UUID64 AddNode(const NodeDataType& data);

	const UUID64 LinkNodes(const UUID64 startNode, const UUID64 endNode, const EdgeMetadataType& metadata = {});
	const UUID64 LinkNodes(const UUID64 id, const UUID64 startNode, const UUID64 endNode, const EdgeMetadataType& metadata = {});

	void RemoveNode(const UUID64 nodeId);
	void RemoveEdge(const UUID64 edgeId);

	const bool DoNodeExist(const UUID64 nodeId) const;

	const GraphEdge<EdgeMetadataType>& GetEdgeFromID(const UUID64 edgeId) const;
	const GraphNode<NodeDataType, EdgeMetadataType>& GetNodeFromID(const UUID64 nodeId) const;

	GraphEdge<EdgeMetadataType>& GetEdgeFromID(const UUID64 edgeId);
	GraphNode<NodeDataType, EdgeMetadataType>& GetNodeFromID(const UUID64 nodeId);

	inline auto& GetNodes() { return m_nodes; }
	inline const auto& GetNodes() const { return m_nodes; }

	inline auto& GetEdges() { return m_edges; }
	inline const auto& GetEdges() const { return m_edges; }

private:
	std::vector<GraphNode<NodeDataType, EdgeMetadataType>> m_nodes;
	std::vector<GraphEdge<EdgeMetadataType>> m_edges;
};

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::AddNode(const UUID64 nodeId, const NodeDataType& data)
{
	auto& newNode = m_nodes.emplace_back(nodeId, data, this);
	return newNode.id;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::AddNode(const NodeDataType& data)
{
	auto& newNode = m_nodes.emplace_back(UUID64{}, data, this);
	return newNode.id;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::LinkNodes(const UUID64 startNodeId, const UUID64 endNodeId, const EdgeMetadataType& metadata)
{
	if (!DoNodeExist(startNodeId) || !DoNodeExist(endNodeId))
	{
		return 0;
	}

	auto& newEdge = m_edges.emplace_back(UUID64{}, startNodeId, endNodeId, metadata);
	
	auto& startNode = GetNodeFromID(startNodeId);
	startNode.edges.emplace_back(newEdge.id);
	
	auto& endNode = GetNodeFromID(endNodeId);
	endNode.edges.emplace_back(newEdge.id);

	return newEdge.id;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::LinkNodes(const UUID64 id, const UUID64 startNodeId, const UUID64 endNodeId, const EdgeMetadataType& metadata)
{
	if (!DoNodeExist(startNodeId) || !DoNodeExist(endNodeId))
	{
		return 0;
	}

	auto& newEdge = m_edges.emplace_back(id, startNodeId, endNodeId, metadata);

	auto& startNode = GetNodeFromID(startNodeId);
	startNode.edges.emplace_back(newEdge.id);

	auto& endNode = GetNodeFromID(endNodeId);
	endNode.edges.emplace_back(newEdge.id);

	return newEdge.id;
}

template<typename NodeDataType, typename EdgeMetadataType>
void Graph<NodeDataType, EdgeMetadataType>::RemoveNode(const UUID64 nodeId)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& node) { return node.id == nodeId; });
	if (it == m_nodes.end())
	{
		return;
	}

	auto node = *it;

	for (int32_t i = static_cast<int32_t>(node.edges.size()) - 1; i >= 0; --i)
	{
		RemoveEdge(node.edges.at(i));
	}

	m_nodes.erase(it);
}

template<typename NodeDataType, typename EdgeMetadataType>
void Graph<NodeDataType, EdgeMetadataType>::RemoveEdge(const UUID64 edgeId)
{
	auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const auto& edge) { return edge.id == edgeId; });
	if (it == m_edges.end())
	{
		return;
	}

	auto edge = *it;

	auto startNode = GetNodeFromID(edge.startNode);
	auto endNode = GetNodeFromID(edge.endNode);

	startNode.RemoveEdge(edgeId);
	endNode.RemoveEdge(edgeId);

	m_edges.erase(it);
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const bool Graph<NodeDataType, EdgeMetadataType>::DoNodeExist(const UUID64 nodeId) const
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& node) { return node.id == nodeId; });
	return it != m_nodes.end();
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const GraphEdge<EdgeMetadataType>& Graph<NodeDataType, EdgeMetadataType>::GetEdgeFromID(const UUID64 edgeId) const
{
	auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const auto& edge) { return edge.id == edgeId; });
	if (it == m_edges.end())
	{
		static GraphEdge<EdgeMetadataType> nullEdge;
		return nullEdge;
	}

	return *it;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const GraphNode<NodeDataType, EdgeMetadataType>& Graph<NodeDataType, EdgeMetadataType>::GetNodeFromID(const UUID64 nodeId) const
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& node) { return node.id == nodeId; });
	if (it == m_nodes.end())
	{
		static GraphNode<NodeDataType, EdgeMetadataType> nullNode{ 0, NodeDataType{}, nullptr };
		return nullNode;
	}

	return *it;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline GraphEdge<EdgeMetadataType>& Graph<NodeDataType, EdgeMetadataType>::GetEdgeFromID(const UUID64 edgeId)
{
	auto it = std::find_if(m_edges.begin(), m_edges.end(), [&](const auto& edge) { return edge.id == edgeId; });
	if (it == m_edges.end())
	{
		static GraphEdge<EdgeMetadataType> nullEdge;
		return nullEdge;
	}

	return *it;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline GraphNode<NodeDataType, EdgeMetadataType>& Graph<NodeDataType, EdgeMetadataType>::GetNodeFromID(const UUID64 nodeId)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& node) { return node.id == nodeId; });
	if (it == m_nodes.end())
	{
		static GraphNode<NodeDataType, EdgeMetadataType> nullNode{ 0, NodeDataType{}, nullptr };
		return nullNode;
	}

	return *it;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const std::vector<UUID64> GraphNode<NodeDataType, EdgeMetadataType>::GetInputEdges() const
{
	auto view = edges | std::views::filter([&](UUID64 edgeId)
	{
		const auto& edge = m_graph->GetEdgeFromID(edgeId);
		return edge.endNode == id;
	});

	return std::vector<UUID64>{ view.begin(), view.end() };
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const GraphEdge<EdgeMetadataType>& GraphNode<NodeDataType, EdgeMetadataType>::GetEdgeFromID(const UUID64 edgeId) const
{
	return m_graph->GetEdgeFromID(edgeId);
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const GraphNode<NodeDataType, EdgeMetadataType>& GraphNode<NodeDataType, EdgeMetadataType>::GetNodeFromID(const UUID64 nodeId) const
{
	return m_graph->GetNodeFromID(nodeId);
}

template<typename NodeDataType, typename EdgeMetadataType>
inline void GraphNode<NodeDataType, EdgeMetadataType>::RemoveEdge(const UUID64 edgeId)
{
	auto it = std::find_if(edges.begin(), edges.end(), [&](const auto& edge) { return edge == edgeId; });
	if (it == edges.end())
	{
		return;
	}

	edges.erase(it);
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const std::vector<UUID64> GraphNode<NodeDataType, EdgeMetadataType>::GetOutputEdges() const
{
	auto view = edges | std::views::filter([&](UUID64 edgeId)
	{
		const auto& edge = m_graph->GetEdgeFromID(edgeId);
		return edge.startNode == id;
	});

	return std::vector<UUID64>{ view.begin(), view.end() };
}
