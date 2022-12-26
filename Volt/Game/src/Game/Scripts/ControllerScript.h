#pragma once

#include <Volt/Scripting/ScriptBase.h>
#include <Volt/Events/KeyEvent.h>

class ControllerScript : public Volt::ScriptBase
{
public:
	ControllerScript(Volt::Entity entity);
	~ControllerScript() override = default;

	void OnStart() override;
	void OnEvent(Volt::Event& e) override;
	void OnUpdate(float aDeltaTime) override;

	static Ref<ScriptBase> Create(Volt::Entity aEntity) { return CreateRef<ControllerScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{760B9F1E-89AE-4711-8CFF-299BD35DB64C}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	gem::vec3 GetWorldPosFromMouse();
	Volt::Entity GetCameraEntity();

	gem::vec2 myViewportMousePos;
	gem::vec2 myViewportSize;

	gem::vec3 myTarget;
	gem::vec3 myDirection;

	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
};