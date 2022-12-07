#include <sbpch.h>

#include "EditorCommandStack.h"
#include "EditorCommand.h"	

EditorCommandStack& EditorCommandStack::GetInstance()
{
	static EditorCommandStack instance; 

	return instance;
}

void EditorCommandStack::PushUndo(Ref<EditorCommand> cmd, bool redoAction)
{
	myUndoStack.push_back(cmd);

	if (!redoAction)
	{
		for (int i = 0; myRedoStack.size(); i++)
		{
			myRedoStack.erase(myRedoStack.begin() + myRedoStack.size() - 1);
		}
	}
}

void EditorCommandStack::PushRedo(Ref<EditorCommand> cmd)
{
	myRedoStack.push_back(cmd);
}

void EditorCommandStack::Undo()
{
	if (myUndoStack.empty())
	{
		return;
	}

	myUndoStack[myUndoStack.size() - 1]->Undo();
	myUndoStack.erase(myUndoStack.begin() + myUndoStack.size() - 1);
}

void EditorCommandStack::Redo()
{
	if (myRedoStack.empty())
	{
		return;
	}

	myRedoStack[myRedoStack.size() - 1]->Redo();
	myRedoStack.erase(myRedoStack.begin() + myRedoStack.size() - 1);
}

void EditorCommandStack::Update(const int aMaxStackSize)
{
	if (myUndoStack.size() > aMaxStackSize)
	{
		for (size_t i = myUndoStack.size(); i > aMaxStackSize; i--)
		{
			myUndoStack.erase(myUndoStack.begin());
		}
	}
}
