#pragma once

#include <Volt/Scripting/Script.h>
#include <Volt/Events/KeyEvent.h>

class WanderScript : public Volt::Script
{
public:
	WanderScript(Volt::Entity entity);
	~WanderScript() override = default;

	void OnStart() override;
	void OnEvent(Volt::Event& e) override;
	void OnUpdate(float aDeltaTime) override;

	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<WanderScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{4A293045-3D97-4215-906D-3D44D422CEE2}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	gem::vec2 myViewportMousePos;
	gem::vec2 myViewportSize;

	gem::vec3 myTarget;
	gem::vec3 myDirection;
};