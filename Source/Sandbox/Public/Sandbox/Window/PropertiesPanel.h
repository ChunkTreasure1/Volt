#pragma once

#include "Sandbox/Window/EditorWindow.h"
#include "Sandbox/EditorCommand.h"
#include "Sandbox/Sandbox.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>


#include <variant>

enum class PropertyEventType
{
	Position,
	Rotation,
	Scale
};

struct PropertyEvent
{
	entt::entity myEntityId;
	PropertyEventType myType;
	std::variant<glm::vec3> myValue;
};

namespace Volt
{
	class SceneRenderer;
}

class PropertiesPanel : public EditorWindow
{
public:
	PropertiesPanel(Ref<Volt::Scene>& currentScene, Ref<Volt::SceneRenderer>& currentSceneRenderer, SceneState& sceneState, const std::string& id);
	void UpdateMainContent() override;

private:
	void AddComponentPopup();

	Ref<Volt::Scene>& myCurrentScene;
	Ref<Volt::SceneRenderer>& myCurrentSceneRenderer;
	SceneState& mySceneState;

	std::string myComponentSearchQuery;
	std::string myScriptSearchQuery;

	bool myActivateComponentSearch = false;
	bool myActivateScriptSearch = false;

	bool myMidEvent = false;
	std::shared_ptr<PropertyEvent> myLastValue;
	glm::vec3 myLastValue2;
	Vector<std::shared_ptr<PropertyEvent>> myUndoList;
	Vector<std::shared_ptr<PropertyEvent>> myRedoList;
	int myMaxEventListSize;
};
