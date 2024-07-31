#pragma once

#include "CoreUtilities/Containers/Vector.h"
#include "CoreUtilities/Containers/StackVector.h"
#include "CoreUtilities/Containers/Map.h"

#include <algorithm>

#include <glm/glm.hpp>


template<typename T>
struct OctreeData
{
	glm::vec3 point;
	T data;
};

template<typename T, size_t MAX_NODE_CAPACITY = 1>
class SparseOctree
{
public:
	typedef size_t SONodeID;
	inline static constexpr SONodeID NullID = ~0u;
	inline static constexpr size_t MaxDepth = 32;

	struct Node
	{
		Node() = default;
		Node(const SONodeID& parentNode, const glm::vec3& iMin, const glm::vec3& iMax)
			: parent(parentNode), min(iMin), max(iMax)
		{ }

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
		vt::map<uint32_t, SONodeID> children;
		Vector<OctreeData<T>> data;
	};

	SparseOctree(const glm::vec3& min, const glm::vec3& max, int32_t maxDepth)
		: m_maxDepth(maxDepth)
	{ 
		m_rootNode = m_nodes.size();
		auto& node = m_nodes.emplace_back();

		node.min = min;
		node.max = max;
	}

	void Insert(const glm::vec3& point, const T& data)
	{
		Insert(m_rootNode, point, data, 0);
	}

	VT_INLINE VT_NODISCARD const Vector<Node>& GetNodes() const { return m_nodes; }

private:
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
	SONodeID m_rootNode = NullID;
};
