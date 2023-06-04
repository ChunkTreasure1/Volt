#pragma once
#include "../../NodeGraph/NodeGraphEditorBackend.h"
#include "BehaviorEditor.h"

class BehaviorGraphBackend : public NodeGraph::EditorBackend
{
public:
	~BehaviorGraphBackend() override = default;
};

class BehaviorPanel : public BehaviorEditor
{
public:
	BehaviorPanel();
	~BehaviorPanel();

};
