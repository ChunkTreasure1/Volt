#pragma once
#include "Volt/Core/UUID.h"
#include "Volt/BehaviorTree/Structs.h"


namespace Volt::BehaviorTree
{
	enum class eNodeKind
	{
		NIL,
		ROOT,
		SEQUENCE,
		SELECTOR,
		DECORATOR,
		LEAF
	};

	class Tree;
	class Node
	{
	public:
		virtual ~Node() = default;
		virtual eNodeStatus Run() = 0;
		UUID64 GetUUID() { return m_uuid; }
		void SetUUID(const UUID64& in_id);
		eNodeKind m_kind = eNodeKind::NIL;
		std::string m_name = "SOMETHING WENT WRONG HERE";
		void SetPos(const std::string& in_data) { m_position = in_data; }
		std::string& GetPos() { return m_position; }

		Tree* m_tree = nullptr;
	protected:
		std::string m_position;
		friend class Tree;
		UUID64 m_uuid;
	};

	// One child
	class Root : public Node
	{
	public:
		Root() { m_kind = eNodeKind::ROOT; m_name = "ROOT"; }
		~Root() override = default;
		eNodeStatus Run() override;
	private:
	};

	// Many children
	class Sequence : public Node
	{
	public:
		Sequence() { m_kind = eNodeKind::SEQUENCE; m_name = "SEQUENCE"; }
		~Sequence() override = default;
		eNodeStatus Run() override;
	private:
	};


	// Many children
	class Selector : public Node
	{
	public:
		Selector() { m_kind = eNodeKind::SELECTOR; m_name = "SELECTOR"; }
		~Selector() override = default;
		eNodeStatus Run() override;
	private:
	};


	// One child

	class Decorator : public Node
	{
	public:
		Decorator() { m_kind = eNodeKind::DECORATOR; m_name = "DECORATOR"; m_if = ""; }
		~Decorator() override = default;
		virtual eNodeStatus Run() override;
		eDecoratorType m_type = eDecoratorType::SUCCEEDER;
		std::string m_if = "";
	private:
	};

	class Leaf : public Node
	{
	public:
		Leaf() { m_kind = eNodeKind::LEAF; m_name = "LEAF"; }
		~Leaf() override = default;
		eNodeStatus Run();
		char bfr[200] = "";
		unsigned int size = 200;
		std::string m_monoScriptFunctonName;
	protected:
		//std::function<eNodeStatus()> m_func;
	};
}
