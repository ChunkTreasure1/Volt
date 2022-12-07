#pragma once

class EditorCommand
{
public:
	virtual ~EditorCommand() {}

	virtual void Execute() = 0;
	virtual void Undo() = 0;
	virtual void Redo() = 0;
};