#pragma once

#include <Volt/Core/Base.h>
#include <stack>
#include "EditorCommand.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Components/Components.h"

#include <tuple>

class EditorCommandStack
{
public:
	static EditorCommandStack& GetInstance();
	static void PushUndo(Ref<EditorCommand> cmd, bool redoAction = false);
	static void PushRedo(Ref<EditorCommand> cmd);
	static void Undo();
	static void Redo();
	static void Clear();
	void Update(const int aMaxStackSize);

private:
	inline static std::vector<Ref<EditorCommand>> myUndoStack;
	inline static std::vector<Ref<EditorCommand>> myRedoStack;
};

template<typename T>
struct ValueCommand : EditorCommand
{
	ValueCommand(T* aValueAdress, T aPreviousValue) : myValueAdress(aValueAdress), myPreviousValue(aPreviousValue) {}
	void Execute() override {}

	void Undo() override
	{
		Ref<ValueCommand<T>> command = CreateRef<ValueCommand<T>>(myValueAdress, *myValueAdress);
		EditorCommandStack::GetInstance().PushRedo(command);
		*myValueAdress = myPreviousValue;
	}

	void Redo() override
	{
		Ref<ValueCommand<T>> command = CreateRef<ValueCommand<T>>(myValueAdress, *myValueAdress);
		EditorCommandStack::GetInstance().PushUndo(command, true);
		*myValueAdress = myPreviousValue;
	}

private:
	T* myValueAdress;
	const T myPreviousValue;
};

struct GizmoCommand : EditorCommand
{
	struct GizmoData
	{
		glm::vec3* positionAdress;
		glm::quat* rotationAdress;
		glm::vec3* scaleAdress;

		glm::vec3 previousPositionValue;
		glm::quat previousRotationValue;
		glm::vec3 previousScaleValue;

		Weak<Volt::Scene> scene;
		Wire::EntityId id;
	};

	GizmoCommand(GizmoData aGizmoData)
		: myPositionAdress(aGizmoData.positionAdress),
		myRotationAdress(aGizmoData.rotationAdress),
		myScaleAdress(aGizmoData.scaleAdress),
		myPreviousPositionValue(aGizmoData.previousPositionValue),
		myPreviousRotationValue(aGizmoData.previousRotationValue),
		myPreviousScaleValue(aGizmoData.previousScaleValue), myID(aGizmoData.id), myScene(aGizmoData.scene)
	{
	}
	void Execute() override {}

	void Undo() override
	{
		GizmoData data;
		data.positionAdress = myPositionAdress;
		data.rotationAdress = myRotationAdress;
		data.scaleAdress = myScaleAdress;
		data.previousPositionValue = *myPositionAdress;

		data.previousRotationValue = *myRotationAdress;
		data.previousScaleValue = *myScaleAdress;
		data.id = myID;
		data.scene = myScene;

		Ref<GizmoCommand> command = CreateRef<GizmoCommand>(data);
		EditorCommandStack::GetInstance().PushRedo(command);

		*myPositionAdress = myPreviousPositionValue;
		*myRotationAdress = myPreviousRotationValue;
		*myScaleAdress = myPreviousScaleValue;

		if (!myScene)
		{
			myScene->InvalidateEntityTransform(myID);
		}
	}

	void Redo() override
	{
		GizmoData data;
		data.positionAdress = myPositionAdress;
		data.rotationAdress = myRotationAdress;
		data.scaleAdress = myScaleAdress;
		data.previousPositionValue = *myPositionAdress;
		data.previousRotationValue = *myRotationAdress;
		data.previousScaleValue = *myScaleAdress;
		data.id = myID;
		data.scene = myScene;

		Ref<GizmoCommand> command = CreateRef<GizmoCommand>(data);
		EditorCommandStack::GetInstance().PushUndo(command, true);

		*myPositionAdress = myPreviousPositionValue;
		*myRotationAdress = myPreviousRotationValue;
		*myScaleAdress = myPreviousScaleValue;
	
		if (!myScene)
		{
			myScene->InvalidateEntityTransform(myID);
		}
	}

private:
	glm::vec3* myPositionAdress;
	glm::quat* myRotationAdress;
	glm::vec3* myScaleAdress;
	const glm::vec3 myPreviousPositionValue;
	const glm::quat myPreviousRotationValue;
	const glm::vec3 myPreviousScaleValue;

	Wire::EntityId myID;
	Weak<Volt::Scene> myScene;
};

struct MultiGizmoCommand : EditorCommand
{
	MultiGizmoCommand(Weak<Volt::Scene> scene, const std::vector<std::pair<Wire::EntityId, Volt::TransformComponent>>& entities)
		: myPreviousTransforms(entities), myScene(scene)
	{
	}

	void Execute() override {}

	void Undo() override
	{
		if (myScene)
		{
			return;
		}

		std::vector<std::pair<Wire::EntityId, Volt::TransformComponent>> currentTransforms;

		for (const auto& [id, oldComp] : myPreviousTransforms)
		{
			Volt::Entity entity{ id, myScene.Get() };
			Volt::TransformComponent transformComponent = entity.GetComponent<Volt::TransformComponent>();

			currentTransforms.emplace_back(id, transformComponent);
			entity.GetComponent<Volt::TransformComponent>() = oldComp;

			myScene->InvalidateEntityTransform(id);
		}

		Ref<MultiGizmoCommand> command = CreateRef<MultiGizmoCommand>(myScene, currentTransforms);
		EditorCommandStack::GetInstance().PushRedo(command);
	}

	void Redo() override
	{
		if (myScene)
		{
			return;
		}

		std::vector<std::pair<Wire::EntityId, Volt::TransformComponent>> currentTransforms;

		for (const auto& [id, oldComp] : myPreviousTransforms)
		{
			Volt::Entity entity{ id, myScene.Get() };
			Volt::TransformComponent transformComponent = entity.GetComponent<Volt::TransformComponent>();

			currentTransforms.emplace_back(id, transformComponent);
			entity.GetComponent<Volt::TransformComponent>() = oldComp;

			myScene->InvalidateEntityTransform(id);
		}

		Ref<MultiGizmoCommand> command = CreateRef<MultiGizmoCommand>(myScene, currentTransforms);
		EditorCommandStack::GetInstance().PushUndo(command);
	}

private:
	std::vector<std::pair<Wire::EntityId, Volt::TransformComponent>> myPreviousTransforms;
	Weak<Volt::Scene> myScene;
};

enum class ObjectStateAction
{
	Create,
	Delete
};

struct ObjectStateCommand : EditorCommand
{
	ObjectStateCommand(std::vector<Volt::Entity> anEntityList, ObjectStateAction anAction) :
		myEntities(anEntityList), myAction(anAction)
	{
		if (anAction == ObjectStateAction::Delete)
		{
			CopyDataToRegistry();
		}
	}

	ObjectStateCommand(Volt::Entity anEntity, ObjectStateAction anAction) :
		myAction(anAction)
	{
		std::vector<Volt::Entity> list;
		list.push_back(anEntity);
		myEntities = list;
	}

	void Execute() override
	{}

	void Undo() override
	{
		if (myAction == ObjectStateAction::Create)
		{
			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(myEntities, ObjectStateAction::Delete);

			EditorCommandStack::PushRedo(command);

			for (int i = 0; i < myEntities.size(); i++)
			{
				myEntities[i].GetScene()->RemoveEntity(myEntities[i]);
			}
		}
		else if (myAction == ObjectStateAction::Delete)
		{
			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(myEntities, ObjectStateAction::Create);
			EditorCommandStack::PushRedo(command);

			for (int i = 0; i < myEntities.size(); i++)
			{
				myEntities[i].GetScene()->GetRegistry().AddEntity(myEntities[i].GetId());
				myEntities[i].Copy(myRegistry, myEntities[i].GetScene()->GetRegistry(), myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetId(), myEntities[i].GetId());
			}
		}
	}

	void Redo() override
	{
		if (myAction == ObjectStateAction::Create)
		{
			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(myEntities, ObjectStateAction::Delete);

			EditorCommandStack::PushUndo(command, true);

			for (int i = 0; i < myEntities.size(); i++)
			{
				myEntities[i].GetScene()->RemoveEntity(myEntities[i]);
			}
		}
		else if (myAction == ObjectStateAction::Delete)
		{
			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(myEntities, ObjectStateAction::Create);
			EditorCommandStack::PushUndo(command, true);

			for (int i = 0; i < myEntities.size(); i++)
			{
				myEntities[i].GetScene()->GetRegistry().AddEntity(myEntities[i].GetId());
				myEntities[i].Copy(myRegistry, myEntities[i].GetScene()->GetRegistry(), myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetId(), myEntities[i].GetId());
			}
		}
	}

private:
	void CopyDataToRegistry()
	{
		for (int i = 0; i < myEntities.size(); i++)
		{
			Volt::Entity::Copy(myEntities[i].GetScene()->GetRegistry(), myRegistry, myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetScene()->GetScriptFieldCache(), myEntities[i].GetId(), myEntities[i].GetId());
		}
	}

	Wire::Registry myRegistry;
	std::vector<Volt::Entity> myEntities;
	ObjectStateAction myAction;
};

enum class ParentingAction
{
	Parent,
	Unparent
};

struct ParentChildData
{
	Volt::Entity myParent;
	Volt::Entity myChild;
};

struct ParentingCommand : EditorCommand
{
	ParentingCommand(std::vector<Ref<ParentChildData>> aData, ParentingAction anAction) :
		myData(aData), myAction(anAction)
	{}

	void Execute() override
	{}

	void Undo() override
	{
		if (myAction == ParentingAction::Parent)
		{
			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(myData, ParentingAction::Unparent);
			EditorCommandStack::PushRedo(command);

			for (int i = 0; i < myData.size(); i++)
			{
				myData[i]->myChild.GetScene()->UnparentEntity(myData[i]->myChild);
			}
		}
		else if (myAction == ParentingAction::Unparent)
		{
			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(myData, ParentingAction::Parent);
			EditorCommandStack::PushRedo(command);

			for (int i = 0; i < myData.size(); i++)
			{
				myData[i]->myChild.GetScene()->ParentEntity(myData[i]->myParent, myData[i]->myChild);
			}
		}
	}

	void Redo() override
	{
		if (myAction == ParentingAction::Parent)
		{
			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(myData, ParentingAction::Unparent);
			EditorCommandStack::PushUndo(command, true);

			for (int i = 0; i < myData.size(); i++)
			{
				myData[i]->myChild.GetScene()->UnparentEntity(myData[i]->myChild);
			}
		}
		else if (myAction == ParentingAction::Unparent)
		{
			Ref<ParentingCommand> command = CreateRef<ParentingCommand>(myData, ParentingAction::Parent);
			EditorCommandStack::PushUndo(command, true);

			for (int i = 0; i < myData.size(); i++)
			{
				myData[i]->myChild.GetScene()->ParentEntity(myData[i]->myParent, myData[i]->myChild);
			}
		}
	}

private:
	std::vector<Ref<ParentChildData>> myData;
	ParentingAction myAction;
};
