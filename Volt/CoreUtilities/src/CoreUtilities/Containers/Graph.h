#pragma once

#include "CoreUtilities/UUID.h"

#include <vector>
#include <unordered_map>

template<typename MetaDataType>
struct Edge
{
	UUID64 id = 0;
	UUID64 startNode = 0;
	UUID64 endNode = 0;

	MetaDataType metaDataType{};
};

template<typename NodeDataType>
struct Node
{
	UUID64 id = 0;
	NodeDataType nodeData{};

	std::vector<UUID64> edges;
};

template<typename NodeDataType, typename EdgeMetadataType>
class Graph
{
public:
	Graph();
	~Graph();

	const UUID64 AddNode(const NodeDataType& data);
	const UUID64 LinkNodes(const UUID64 startNode, const UUID64 endNode, const EdgeMetadataType& metadata = {});

	void RemoveNode(const UUID64 nodeId);
	void RemoveEdge(const UUID64 edgeId);

	const bool DoNodeExist(const UUID64 nodeId);

private:
	std::vector<Node<NodeDataType>> m_nodes;
	std::vector<Edge<EdgeMetadataType>> m_edges;
};

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::AddNode(const NodeDataType& data)
{
	auto& newNode = m_nodes.emplace_back(UUID64{}, data);
	return newNode.id;
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const UUID64 Graph<NodeDataType, EdgeMetadataType>::LinkNodes(const UUID64 startNode, const UUID64 endNode, const EdgeMetadataType& metadata)
{
	if (!DoNodeExist(startNode) || !DoNodeExist(endNode))
	{
		return 0;
	}

	auto& newEdge = m_edges.emplace_back(UUID64{}, startNode, endNode, metadata);
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

	m_edges.erase(it);
}

template<typename NodeDataType, typename EdgeMetadataType>
inline const bool Graph<NodeDataType, EdgeMetadataType>::DoNodeExist(const UUID64 nodeId)
{
	auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](const auto& node) { return node.id == nodeId; });
	return it != m_nodes.end();
}
