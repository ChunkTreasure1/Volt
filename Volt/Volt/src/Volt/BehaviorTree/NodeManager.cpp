#include "vtpch.h"
#include "NodeManager.h"

namespace Volt::BehaviorTree
{
	Ref<Node> NodeManager::GetNodeFromUUID(const UUID& in_uuid) const
	{
		if (m_nodes.find(in_uuid) == m_nodes.end())
			return nullptr;
		return m_nodes.at(in_uuid);
	}

	const std::vector<Link>& NodeManager::GetLinksFromUUID(const UUID& in_uuid)
	{
		if (m_links.find(in_uuid) == m_links.end())
			return Nil::linkVec;
		return m_links[in_uuid];
	}

	const Link& NodeManager::GetLink(const UUID& in_uuid)
	{
		if (m_linkIDs.find(in_uuid) == m_linkIDs.end())
			return Nil::link;
		return m_linkIDs[in_uuid];
	}

	NodeManager::NodeManager()
	{
	}

	NodeManager::~NodeManager()
	{

	}

	bool NodeManager::RegisterNode(const UUID& in_uuid, Ref<Node> in_node)
	{
		if (m_nodes.find(in_uuid) != m_nodes.end())
			return false;
		m_nodes[in_uuid] = in_node;
		return true;
	}

	bool NodeManager::RegisterLink(const UUID& in_parentID, const UUID& in_childID, const UUID& in_linkID)
	{
		if (m_nodes.find(in_parentID) == m_nodes.end())
		{
			assert(false && "Parent node does not exist");
			return false;
		}
		if (m_nodes.find(in_childID) == m_nodes.end())
		{
			assert(false && "Child node does not exist");
			return false;
		}

		// Create link
		Link lnk;

		if (in_linkID == 0)
		{
			while (lnk.m_uuid == Nil::id || m_linkIDs.find(lnk.m_uuid) != m_linkIDs.end())
				lnk.m_uuid = UUID();
		}
		else
		{
			lnk.m_uuid = in_linkID;
		}
		lnk.m_childID = UUID(in_childID);
		lnk.m_parentID = UUID(in_parentID);

		// Add link to link related maps
		m_links[in_parentID].push_back(lnk);
		m_linkIDs.insert({ lnk.m_uuid, lnk });
		return true;
	}

	void NodeManager::UnregisterNode(const UUID& in_uuid)
	{
		assert(m_nodes.find(in_uuid) != m_nodes.end() && "Node does not exist");

		// find all links related to node
		std::vector<UUID> linksToRemove;
		linksToRemove.reserve(10);
		for (const auto& id : m_linkIDs)
		{
			const auto& lnk = GetLink(id.first);
			if (lnk == Nil::link)
			{
				assert(false && "Something went wrong somewhere, link should exist in vector");
				continue;
			}
			if (lnk.m_childID != in_uuid && lnk.m_parentID != in_uuid)
				continue;
			linksToRemove.push_back(lnk.m_uuid);
		}

		// Unregister links and remove node
		for (const auto& rLink : linksToRemove)
			UnregisterLink(rLink);
		m_nodes.erase(m_nodes.find(in_uuid));
	}

	void NodeManager::UnregisterLink(const Link& in_link)
	{
		// Unregister from parent linkMap
		std::vector<Link>& links = m_links[in_link.m_parentID];
		for (int i = 0; i < links.size(); i++)
		{
			if (links[i] == in_link)
			{
				links.erase(links.begin() + i);
			}
		}

		// Unregister from linkIDs
		if (m_linkIDs.find(in_link.m_uuid) != m_linkIDs.end())
			m_linkIDs.erase(m_linkIDs.find(in_link.m_uuid));
	}

	void NodeManager::UnregisterLink(const UUID& in_linkID)
	{
		assert(m_linkIDs.find(in_linkID) != m_linkIDs.end() && "Link id does not exist");
		UnregisterLink(GetLink(in_linkID));
	}
}
