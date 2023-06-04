#pragma once

#include "Sandbox/NodeGraph/NodeGraphEditorBackend.h"

#include <GraphKey/Node.h>
#include <GraphKey/Graph.h>

class NodeGraphCommand
{
public:
	virtual ~NodeGraphCommand() = default;

	virtual void Undo(Ref<GraphKey::Graph>& graph, std::function<void()> reconstructFunc) = 0;
	virtual void Redo(Ref<GraphKey::Graph>& graph, std::function<void()> reconstructFunc) = 0;
};

class NodeGraphGraphKeyCommand : public NodeGraphCommand
{
public:
	NodeGraphGraphKeyCommand(Ref<GraphKey::Graph> currentGraphState)
	{
		myUndoGraphState = CreateRef<GraphKey::Graph>();
		GraphKey::Graph::Copy(currentGraphState, myUndoGraphState);
	}

	~NodeGraphGraphKeyCommand() = default;

	inline void Undo(Ref<GraphKey::Graph>& graph, std::function<void()> reconstructFunc) override
	{
		myRedoGraphState = CreateRef<GraphKey::Graph>();
		GraphKey::Graph::Copy(graph, myRedoGraphState);

		graph = CreateRef<GraphKey::Graph>();
		GraphKey::Graph::Copy(myUndoGraphState, graph);

		reconstructFunc();
	}

	inline void Redo(Ref<GraphKey::Graph>& graph, std::function<void()> reconstructFunc) override
	{
		graph = CreateRef<GraphKey::Graph>();
		GraphKey::Graph::Copy(myRedoGraphState, graph);

		reconstructFunc();
	}

private:
	Ref<GraphKey::Graph> myUndoGraphState;
	Ref<GraphKey::Graph> myRedoGraphState;
};

class NodeGraphCommandStack
{
public:
	NodeGraphCommandStack() = default;
	~NodeGraphCommandStack() = default;

	template<typename T, typename... Args>
	void AddCommand(Args&&...);

	template<typename... Args>
	void Undo(Args&&...);

	template<typename... Args>
	void Redo(Args&&...);

private:
	std::vector<Scope<NodeGraphCommand>> myUndoCommands;
	std::vector<Scope<NodeGraphCommand>> myRedoCommands;
};

template<typename T, typename ...Args>
inline void NodeGraphCommandStack::AddCommand(Args&& ... args)
{
	myUndoCommands.push_back(CreateScope<T>(std::forward<Args>(args)...));
	myRedoCommands.clear();
}

template<typename ...Args>
inline void NodeGraphCommandStack::Undo(Args && ... args)
{
	if (myUndoCommands.empty())
	{
		return;
	}

	myUndoCommands.back()->Undo(std::forward<Args>(args)...);
	myRedoCommands.emplace_back(std::move(myUndoCommands.back()));
	myUndoCommands.pop_back();
}

template<typename ...Args>
inline void NodeGraphCommandStack::Redo(Args && ... args)
{
	if (myRedoCommands.empty())
	{
		return;
	}

	myRedoCommands.back()->Redo(std::forward<Args>(args)...);
	myUndoCommands.emplace_back(std::move(myRedoCommands.back()));
	myRedoCommands.pop_back();
}
