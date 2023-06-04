#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include <Volt/Components/Components.h>
#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>
#include <vector>
#include <variant>
#include "Sandbox/EditorCommand.h"

enum class PropertyEventType
{
	Position,
	Rotation,
	Scale
};

struct PropertyEvent
{
	Wire::EntityId myEntityId;
	PropertyEventType myType;
	std::variant<gem::vec3> myValue;
};

namespace Volt
{
	class SceneRenderer;
}

class PropertiesPanel : public EditorWindow
{
public:
	PropertiesPanel(Ref<Volt::Scene>& currentScene, Ref<Volt::SceneRenderer>& currentSceneRenderer, const std::string& id);
	void UpdateMainContent() override;

private:
	void AddComponentPopup();
	void AddMonoScriptPopup();
	void AcceptMonoDragDrop();

	void DrawMonoScript(Volt::MonoScriptEntry& scriptEntry, const Wire::EntityId& entity, Wire::Registry& registry, const Wire::ComponentRegistry::RegistrationInfo& registryInfo);
	void DrawMonoProperties(Wire::Registry& registry, const Wire::ComponentRegistry::RegistrationInfo& registryInfo, Volt::MonoScriptEntry& scriptEntry);
	void DrawGraphKeyProperties(const Wire::EntityId id, Volt::VisualScriptingComponent& comp);

	Ref<Volt::Scene>& myCurrentScene;
	Ref<Volt::SceneRenderer>& myCurrentSceneRenderer;

	std::string myComponentSearchQuery;
	std::string myScriptSearchQuery;

	bool myActivateComponentSearch = false;
	bool myActivateScriptSearch = false;

	bool myMidEvent = false;
	std::shared_ptr<PropertyEvent> myLastValue;
	gem::vec3 myLastValue2;
	std::vector<std::shared_ptr<PropertyEvent>> myUndoList;
	std::vector<std::shared_ptr<PropertyEvent>> myRedoList;
	int myMaxEventListSize;
};
