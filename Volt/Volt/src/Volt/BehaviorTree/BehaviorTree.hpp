#pragma once
#include "NodeManager.h"
#include "Nodes/BehaviorTreeNode.h"
#include "Volt/Asset/Asset.h"

#include "Volt/Scene/Entity.h"

#include <filesystem>
#include <vector>

namespace Volt::BehaviorTree
{
	class Tree : public Asset
	{
	public:
		Tree();
		Tree(Tree& in_tree);
		~Tree();

		eNodeStatus Run() { return m_nodeManger.GetNodeFromUUID(m_root)->Run(); }
		NodeManager& GetNodeManager() { return m_nodeManger; } 

		UUID GetRoot() { return m_root; }
		void SetRoot(UUID in_uuid) { m_root = in_uuid; }
		void ResetRoot() { m_root = m_startRoot; }

		void SetEntity(const Volt::Entity& in_entity) { m_entity = in_entity; }
		Entity GetEntity() { return m_entity; }
		void SetStartRoot(UUID in_uuid) { m_startRoot = in_uuid; }
		void FullReset();
		void ClearRootLink() { m_nodeManger.m_links[m_root].clear(); }

		template<typename _T>
		UUID CreateNode();
		template<typename _T>
		UUID CreateNode(const UUID& in_id);

		static AssetType GetStaticType() { return AssetType::BehaviorGraph; }
		AssetType GetType() override { return AssetType::BehaviorGraph; }

	private:
		friend class BehaviorTreeImporter;
		friend class BehaviorEditor;
		NodeManager m_nodeManger;

		Volt::Entity m_entity = Volt::Entity::Null();

		UUID m_root = 0;
		UUID m_startRoot = 0;
	};

	inline Tree::Tree()
	{
		m_root = 1;
		auto rootID = CreateNode<Root>(m_root);
		//m_nodeManger.GetNodeFromUUID(rootID)->SetUUID(m_root);
		m_startRoot = m_root;
	}

	inline Tree::Tree(Tree& in_tree)
	{
		m_root = 1;
		auto rootID = CreateNode<Root>();
		m_nodeManger.GetNodeFromUUID(rootID)->SetUUID(m_root);
		m_startRoot = m_root;

		for (auto _link : in_tree.m_nodeManger.m_linkIDs)
		{
			m_nodeManger.m_linkIDs.insert(_link);
		}
		for (auto _pair : in_tree.m_nodeManger.m_links)
		{
			for (const auto& _link : _pair.second)
			{
				m_nodeManger.m_links[_pair.first].push_back(_link);
			}
		}
		for (auto _pair : in_tree.m_nodeManger.m_nodes)
		{
			switch (_pair.second->m_kind)
			{
				case eNodeKind::LEAF:
					CreateNode<Leaf>(_pair.first);
					reinterpret_cast<Leaf*>(m_nodeManger.GetNodeFromUUID(_pair.first).get())->m_monoScriptFunctonName = reinterpret_cast<Leaf*>(_pair.second.get())->m_monoScriptFunctonName;
					break;
				case eNodeKind::DECORATOR:
					CreateNode<Decorator>(_pair.first);
					reinterpret_cast<Decorator*>(m_nodeManger.GetNodeFromUUID(_pair.first).get())->m_type = reinterpret_cast<Decorator*>(_pair.second.get())->m_type;
					reinterpret_cast<Decorator*>(m_nodeManger.GetNodeFromUUID(_pair.first).get())->m_if = reinterpret_cast<Decorator*>(_pair.second.get())->m_if;
					break;
				case eNodeKind::SELECTOR:
					CreateNode<Selector>(_pair.first);
					break;
				case eNodeKind::SEQUENCE:
					CreateNode<Sequence>(_pair.first);
					break;
				default:
					break;
			}
			m_nodeManger.GetNodeFromUUID(_pair.first)->m_position = _pair.second->m_position;
		}
	}

	inline Tree::~Tree()
	{
	}

	inline void Tree::FullReset()
	{
		// #MAX_TODO# implement
	}

	template<typename _T>
	inline UUID Tree::CreateNode()
	{
		static_assert(std::derived_from<_T, Node> && "Bad type");

		Ref<_T> node = CreateRef<_T>();
		while (!m_nodeManger.RegisterNode(node->GetUUID(), node))
			node = CreateRef<_T>();
		node->m_tree = this;
		return node->GetUUID();
	}

	template<typename _T>
	inline UUID Tree::CreateNode(const UUID& in_id)
	{
		static_assert(std::derived_from<_T, Node> && "Bad type");

		if (m_nodeManger.GetNodeFromUUID(in_id))
			return 0;
		Ref<_T> node = CreateRef<_T>();
		node->m_tree = this;
		node->m_uuid = in_id;
		m_nodeManger.RegisterNode(node->GetUUID(), node);
		return node->GetUUID();
	}
}
