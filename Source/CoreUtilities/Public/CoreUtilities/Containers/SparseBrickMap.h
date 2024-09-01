#pragma once

#include "CoreUtilities/Containers/Vector.h"
#include "CoreUtilities/Containers/StackVector.h"
#include "CoreUtilities/Containers/Map.h"
#include "CoreUtilities/Math/Math.h"

#include <algorithm>

#include <glm/glm.hpp>


template<typename T>
struct OctreeData
{
	glm::vec3 point;
	T data;
};

template<typename T, size_t MAX_NODE_CAPACITY = 1, size_t BRICK_SIZE = 8>
class SparseBrickMap
{
public:
	typedef size_t SONodeID;
	inline static constexpr SONodeID NullID = ~0u;
	inline static constexpr size_t MaxDepth = 32;

	struct BrickVoxel
	{
		glm::vec3 point;
		T data;

		bool hasData = false;
	};

	struct Brick
	{
		BrickVoxel data[BRICK_SIZE * BRICK_SIZE * BRICK_SIZE];
	};

	struct Node
	{
		Node() = default;
		Node(const SONodeID& parentNode, const glm::vec3& iMin, const glm::vec3& iMax)
			: parent(parentNode), min(iMin), max(iMax)
		{
		}

		VT_INLINE VT_NODISCARD bool Contains(const glm::vec3& p) const
		{
			return (p.x >= min.x && p.x <= max.x &&
					p.y >= min.y && p.y <= max.y &&
					p.z >= min.z && p.z <= max.z);
		}

		VT_INLINE VT_NODISCARD int32_t GetOctant(const glm::vec3& point) const
		{
			int index = 0;
			glm::vec3 mid = (min + max) / 2.0f;
			if (point.x > mid.x) index |= 1;
			if (point.y > mid.y) index |= 2;
			if (point.z > mid.z) index |= 4;
			return index;
		}

		glm::vec3 min;
		glm::vec3 max;

		SONodeID parent = NullID;
		SONodeID brickId = NullID;

		vt::map<uint32_t, SONodeID> children;
		Vector<OctreeData<T>> data;
	};

	SparseBrickMap(const glm::vec3& min, const glm::vec3& max, int32_t maxDepth)
		: m_maxDepth(maxDepth)
	{
		m_rootNode = m_nodes.size();
		auto& node = m_nodes.emplace_back();

		node.min = min;
		node.max = max;
	}

	SparseBrickMap(const SparseBrickMap& other)
	{
		m_maxDepth = other.m_maxDepth;
		m_nodes = other.m_nodes;
		m_bricks = other.m_bricks;
		m_rootNode = other.m_rootNode;
	}

	SparseBrickMap(SparseBrickMap&&) = delete;
	SparseBrickMap() = default;

	SparseBrickMap& operator=(const SparseBrickMap& other)
	{
		if (this != &other)
		{
			m_maxDepth = other.m_maxDepth;
			m_nodes = other.m_nodes;
			m_bricks = other.m_bricks;
			m_rootNode = other.m_rootNode;
		}

		return *this;
	}

	void Insert(const glm::vec3& point, const T& data)
	{
		Insert(m_rootNode, point, data, 0);
	}

	void BuildBricks();

	void Traverse(const std::function<void(const Node& node, const SparseBrickMap& brickMap)>& execFunc, int32_t maxDepth = -1) const
	{
		VT_ASSERT(execFunc != nullptr);
		VT_ASSERT(m_rootNode != NullID);

		Traverse(execFunc, m_nodes.at(m_rootNode), maxDepth);
	}

	VT_INLINE VT_NODISCARD const Vector<Node>& GetNodes() const { return m_nodes; }
	VT_INLINE VT_NODISCARD const Brick& GetBrick(SONodeID brickId) const { return m_bricks.at(brickId); }
	VT_INLINE VT_NODISCARD const bool IsValid() const { return m_rootNode != NullID; }

private:
	void Traverse(const std::function<void(const Node& node, const SparseBrickMap& brickMap)>& execFunc, const Node& currentNode, int32_t maxDepth) const
	{
		if (maxDepth == 0)
		{
			return;
		}

		execFunc(currentNode, *this);

		for (const auto& [index, nodeId] : currentNode.children)
		{
			Traverse(execFunc, m_nodes.at(nodeId), maxDepth - 1);
		}
	}

	void Insert(SONodeID nodeId, const glm::vec3& point, const T& data, int32_t depth)
	{
		if (!m_nodes.at(nodeId).Contains(point))
		{
			return;
		}

		if (depth == m_maxDepth || (m_nodes.at(nodeId).data.size() < MAX_NODE_CAPACITY && m_nodes.at(nodeId).children.empty()))
		{
			m_nodes.at(nodeId).data.emplace_back(point, data);
		}
		else
		{
			if (m_nodes.at(nodeId).children.empty())
			{
				SplitNode(nodeId);
			}

			int32_t index = m_nodes.at(nodeId).GetOctant(point);
			if (!m_nodes.at(nodeId).children.contains(index))
			{
				CreateChild(nodeId, index, point);
			}

			Insert(m_nodes.at(nodeId).children[index], point, data, depth + 1);
		}
	}

	void SplitNode(SONodeID nodeId)
	{
		for (const auto& v : m_nodes.at(nodeId).data)
		{
			int32_t index = m_nodes.at(nodeId).GetOctant(v.point);
			if (!m_nodes.at(nodeId).children.contains(index))
			{
				CreateChild(nodeId, index, v.point);
			}

			m_nodes.at(m_nodes.at(nodeId).children[index]).data.emplace_back(v);
		}

		m_nodes.at(nodeId).data.clear();
	}

	void CreateChild(SONodeID parent, int32_t index, const glm::vec3& point)
	{
		glm::vec3 min = m_nodes.at(parent).min;
		glm::vec3 max = m_nodes.at(parent).max;
		glm::vec3 mid = (min + max) / 2.f;

		if (index & 1) min.x = mid.x; else max.x = mid.x;
		if (index & 2) min.y = mid.y; else max.y = mid.y;
		if (index & 4) min.z = mid.z; else max.z = mid.z;

		SONodeID nodeId = m_nodes.size();
		m_nodes.emplace_back() = Node(parent, min, max);

		m_nodes.at(parent).children[index] = nodeId;
	}

	int32_t m_maxDepth;
	Vector<Node> m_nodes;
	Vector<Brick> m_bricks;
	SONodeID m_rootNode = NullID;
};

template<typename T, size_t MAX_NODE_CAPACITY, size_t BRICK_SIZE>
inline void SparseBrickMap<T, MAX_NODE_CAPACITY, BRICK_SIZE>::BuildBricks()
{
	Vector<SONodeID> parents;

	// First propagate the data into the leaf nodes and it's parents.
	// From this step we save the parent nodes of each leaf, so we
	// can continue to propagate the data upwards.
	for (auto& node : m_nodes)
	{
		if (!node.children.empty())
		{
			continue;
		}

		bool created = false;
		bool parentCreated = false;

		if (node.brickId == NullID)
		{
			node.brickId = m_bricks.size();
			m_bricks.emplace_back();
			created = true;
		}

		Node* parentNode = nullptr;
		if (node.parent != NullID)
		{
			parentNode = &m_nodes.at(node.parent);
			parents.emplace_back(node.parent);
		}

		if (parentNode && parentNode->brickId == NullID)
		{
			parentNode->brickId = m_bricks.size();
			m_bricks.emplace_back();
			parentCreated = true;
		}

		for (const auto& data : node.data)
		{
			const glm::vec3 localPos = data.point - node.min;
			const glm::uvec3 localOffset = (localPos / (node.max - node.min)) * static_cast<float>(BRICK_SIZE);

			const size_t index = Math::Get1DIndexFrom3DCoord(static_cast<size_t>(localOffset.x), static_cast<size_t>(localOffset.y), static_cast<size_t>(localOffset.z), BRICK_SIZE, BRICK_SIZE);

			if (created)
			{
				m_bricks.at(node.brickId).data[index].point = data.point;
				m_bricks.at(node.brickId).data[index].data = data.data;
				m_bricks.at(node.brickId).data[index].hasData = true;
			}
			else
			{
				if (glm::abs(data.data) < glm::abs(m_bricks.at(node.brickId).data[index].data))
				{
					m_bricks.at(node.brickId).data[index].point = data.point;
					m_bricks.at(node.brickId).data[index].data = data.data;
					m_bricks.at(node.brickId).data[index].hasData = true;
				}
			}

			if (parentNode)
			{
				const glm::vec3 parentLocalPos = data.point - parentNode->min;
				const glm::uvec3 parentLocalOffset = (parentLocalPos / (parentNode->max - parentNode->min)) * static_cast<float>(BRICK_SIZE);

				const size_t parentIndex = Math::Get1DIndexFrom3DCoord(static_cast<size_t>(parentLocalOffset.x), static_cast<size_t>(parentLocalOffset.y), static_cast<size_t>(parentLocalOffset.z), BRICK_SIZE, BRICK_SIZE);

				if (parentCreated)
				{
					m_bricks.at(parentNode->brickId).data[parentIndex].point = data.point;
					m_bricks.at(parentNode->brickId).data[parentIndex].data = data.data;
					m_bricks.at(parentNode->brickId).data[parentIndex].hasData = true;
				}
				else
				{
					if (glm::abs(data.data) < glm::abs(m_bricks.at(parentNode->brickId).data[parentIndex].data))
					{
						m_bricks.at(parentNode->brickId).data[parentIndex].point = data.point;
						m_bricks.at(parentNode->brickId).data[parentIndex].data = data.data;
						m_bricks.at(parentNode->brickId).data[parentIndex].hasData = true;
					}
				}
			}
		}
	}

	Vector<SONodeID> newParents;

	Vector<SONodeID>* previousParents = &parents;
	Vector<SONodeID>* currentParents = &newParents;

	// Now propagate the data up the tree.
	while (!previousParents->empty())
	{
		currentParents->clear();

		for (const auto& parentId : (*previousParents))
		{
			auto& node = m_nodes.at(parentId);

			if (node.parent != NullID)
			{
				currentParents->emplace_back(node.parent);
				auto& parentNode = m_nodes.at(node.parent);

				bool parentCreated = false;
				if (parentNode.brickId == NullID)
				{
					parentNode.brickId = m_bricks.size();
					m_bricks.emplace_back();

					parentCreated = true;
				}

				for (const auto& data : m_bricks.at(node.brickId).data)
				{
					const glm::vec3 parentLocalPos = data.point - parentNode.min;
					const glm::uvec3 parentLocalOffset = (parentLocalPos / (parentNode.max - parentNode.min)) * static_cast<float>(BRICK_SIZE);

					const size_t parentIndex = Math::Get1DIndexFrom3DCoord(static_cast<size_t>(parentLocalOffset.x), static_cast<size_t>(parentLocalOffset.y), static_cast<size_t>(parentLocalOffset.z), BRICK_SIZE, BRICK_SIZE);

					if (parentCreated)
					{
						m_bricks.at(parentNode.brickId).data[parentIndex] = data;
					}
					else
					{
						if (glm::abs(data.data) < glm::abs(m_bricks.at(parentNode.brickId).data[parentIndex].data))
						{
							m_bricks.at(parentNode.brickId).data[parentIndex] = data;
						}
					}
				}
			}
		}

		std::swap(currentParents, previousParents);
	}


}
